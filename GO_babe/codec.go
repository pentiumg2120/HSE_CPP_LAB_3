package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"image"
	"image/color"
	"io"
)

const codec = "BABE\n"

var (
	smallBlock = 1
	macroBlock = 2
	encQuality = 70
	encodeBW   bool
)

const (
	channelFlagY  = 1 << 0
	channelFlagCb = 1 << 1
	channelFlagCr = 1 << 2
)

func setBlocksForQuality(quality int) {
	if quality < 0 { quality = 0 }
	if quality > 100 { quality = 100 }
	encQuality = quality
	switch {
	case quality >= 80: smallBlock, macroBlock = 1, 2
	case quality >= 60: smallBlock, macroBlock = 1, 3
	case quality >= 40: smallBlock, macroBlock = 2, 4
	case quality >= 20: smallBlock, macroBlock = 3, 6
	default: smallBlock, macroBlock = 4, 8
	}
}

type bitWriter struct {
	w    io.ByteWriter
	byte byte
	n    uint8
}

func (bw *bitWriter) writeBit(bit bool) error {
	bw.byte <<= 1
	if bit { bw.byte |= 1 }
	bw.n++
	if bw.n == 8 {
		if err := bw.w.WriteByte(bw.byte); err != nil { return err }
		bw.byte, bw.n = 0, 0
	}
	return nil
}

func (bw *bitWriter) flush() error {
	if bw.n > 0 {
		bw.byte <<= 8 - bw.n
		if err := bw.w.WriteByte(bw.byte); err != nil { return err }
		bw.byte, bw.n = 0, 0
	}
	return nil
}

type bitReader struct {
	data []byte
	idx  int
	bit  uint8
}

func (br *bitReader) readBit() (bool, error) {
	if br.idx >= len(br.data) { return false, io.EOF }
	isSet := (br.data[br.idx] & (1 << (7 - br.bit))) != 0
	br.bit++
	if br.bit == 8 { br.bit, br.idx = 0, br.idx+1 }
	return isSet, nil
}

func rgbToYCbCr(r, g, b uint8) (uint8, uint8, uint8) {
	rr, gg, bb := int32(r), int32(g), int32(b)
	Y := (77*rr + 150*gg + 29*bb) >> 8
	Cb := ((-43*rr - 85*gg + 128*bb) >> 8) + 128
	Cr := ((128*rr - 107*gg - 21*bb) >> 8) + 128
	return uint8(Y), uint8(Cb), uint8(Cr)
}

func extractYCbCrPlanes(img image.Image) ([]uint8, []uint8, []uint8, int, int) {
	b := img.Bounds()
	w, h := b.Dx(), b.Dy()
	yP, cbP, crP := make([]uint8, w*h), make([]uint8, w*h), make([]uint8, w*h)
	for y := 0; y < h; y++ {
		for x := 0; x < w; x++ {
			r, g, b, _ := img.At(b.Min.X+x, b.Min.Y+y).RGBA()
			yy, cb, cr := rgbToYCbCr(uint8(r>>8), uint8(g>>8), uint8(b>>8))
			yP[y*w+x], cbP[y*w+x], crP[y*w+x] = yy, cb, cr
		}
	}
	return yP, cbP, crP, w, h
}

func canUseBigBlock(plane []uint8, stride, x0, y0 int) bool {
	var minV, maxV uint8
	for yy := 0; yy < macroBlock; yy++ {
		for xx := 0; xx < macroBlock; xx++ {
			v := plane[(y0+yy)*stride+(x0+xx)]
			if yy == 0 && xx == 0 { minV, maxV = v, v } else {
				if v < minV { minV = v }
				if v > maxV { maxV = v }
			}
		}
	}
	spread := int32(64 - (encQuality/20)*12)
	return int32(maxV-minV) < spread
}

func encodeBlock(plane []uint8, stride, x0, y0, bw, bh int, pw *bitWriter) (uint8, uint8) {
	var sum uint64
	for yy := 0; yy < bh; yy++ {
		for xx := 0; xx < bw; xx++ {
			sum += uint64(plane[(y0+yy)*stride+(x0+xx)])
		}
	}
	thr := uint8(sum / uint64(bw*bh))
	var fgSum, bgSum uint64
	var fgCnt, bgCnt uint32
	for yy := 0; yy < bh; yy++ {
		for xx := 0; xx < bw; xx++ {
			v := plane[(y0+yy)*stride+(x0+xx)]
			isFg := v >= thr
			if pw != nil { pw.writeBit(isFg) }
			if isFg { fgSum += uint64(v); fgCnt++ } else { bgSum += uint64(v); bgCnt++ }
		}
	}
	if fgCnt == 0 || bgCnt == 0 { return thr, thr }
	return uint8(fgSum / uint64(fgCnt)), uint8(bgSum / uint64(bgCnt))
}

func encodeChannel(plane []uint8, stride, w4, h4, fullW, fullH int, useMacro bool) (uint32, []byte, []byte, []byte, []uint8, []uint8) {
	var blockCount uint32
	var sizeBuf, typeBuf, patternBuf bytes.Buffer
	sizeW, typeW, patternW := &bitWriter{w: &sizeBuf}, &bitWriter{w: &typeBuf}, &bitWriter{w: &patternBuf}
	var fgVals, bgVals []uint8

	for my := 0; my < fullH; my += macroBlock {
		for mx := 0; mx < fullW; mx += macroBlock {
			if useMacro && canUseBigBlock(plane, stride, mx, my) {
				fg, bg := encodeBlock(plane, stride, mx, my, macroBlock, macroBlock, patternW)
				fgVals, bgVals = append(fgVals, fg), append(bgVals, bg)
				typeW.writeBit(true); sizeW.writeBit(true); blockCount++
			} else {
				sizeW.writeBit(false)
				for by := 0; by < macroBlock; by += smallBlock {
					for bx := 0; bx < macroBlock; bx += smallBlock {
						var pw *bitWriter
						if smallBlock > 1 { pw = patternW }
						fg, bg := encodeBlock(plane, stride, mx+bx, my+by, smallBlock, smallBlock, pw)
						fgVals, bgVals = append(fgVals, fg), append(bgVals, bg)
						typeW.writeBit(smallBlock > 1); blockCount++
					}
				}
			}
		}
	}
	sizeW.flush(); typeW.flush(); patternW.flush()
	return blockCount, sizeBuf.Bytes(), typeBuf.Bytes(), patternBuf.Bytes(), fgVals, bgVals
}

func Encode(img image.Image, quality int, bwmode bool) ([]byte, error) {
	encodeBW = bwmode
	setBlocksForQuality(quality)
	yP, cbP, crP, w, h := extractYCbCrPlanes(img)
	mask := byte(channelFlagY)
	if !encodeBW { mask |= channelFlagCb | channelFlagCr }

	var raw bytes.Buffer
	bw := bufio.NewWriter(&raw)
	bw.WriteString(codec)
	binary.Write(bw, binary.BigEndian, uint16(smallBlock))
	binary.Write(bw, binary.BigEndian, uint16(macroBlock))
	bw.WriteByte(mask)
	binary.Write(bw, binary.BigEndian, uint32(w))
	binary.Write(bw, binary.BigEndian, uint32(h))

	w4, h4 := (w/smallBlock)*smallBlock, (h/smallBlock)*smallBlock
	fullW, fullH := (w4/macroBlock)*macroBlock, (h4/macroBlock)*macroBlock

	channels := []struct{id int; p []uint8}{{0, yP}}
	if !encodeBW { channels = append(channels, struct{id int; p []uint8}{1, cbP}, struct{id int; p []uint8}{2, crP}) }

	for _, ch := range channels {
		bc, sz, tp, pt, fg, bg := encodeChannel(ch.p, w, w4, h4, fullW, fullH, macroBlock > smallBlock)
		binary.Write(bw, binary.BigEndian, bc)
		binary.Write(bw, binary.BigEndian, uint32(len(sz))); bw.Write(sz)
		binary.Write(bw, binary.BigEndian, uint32(len(tp))); bw.Write(tp)
		binary.Write(bw, binary.BigEndian, uint32(len(pt))); bw.Write(pt)
		binary.Write(bw, binary.BigEndian, uint32(len(fg))); bw.Write(fg)
		binary.Write(bw, binary.BigEndian, uint32(len(bg))); bw.Write(bg)
	}
	bw.Flush()
	return raw.Bytes(), nil
}

func Decode(payload []byte, _ bool) (image.Image, error) {
	pos := len(codec)
	sb := int(binary.BigEndian.Uint16(payload[pos : pos+2])); pos += 2
	mb := int(binary.BigEndian.Uint16(payload[pos : pos+2])); pos += 2
	mask := payload[pos]; pos++
	w := int(binary.BigEndian.Uint32(payload[pos : pos+4])); pos += 4
	h := int(binary.BigEndian.Uint32(payload[pos : pos+4])); pos += 4

	dst := image.NewRGBA(image.Rect(0, 0, w, h))
	planes := make([][]uint8, 3)
	for i := 0; i < 3; i++ {
		if i == 0 || (mask&(1<<i)) != 0 {
			_ = int(binary.BigEndian.Uint32(payload[pos : pos+4])); pos += 4
			szL := int(binary.BigEndian.Uint32(payload[pos : pos+4])); pos += 4
			szB := payload[pos : pos+szL]; pos += szL
			tpL := int(binary.BigEndian.Uint32(payload[pos : pos+4])); pos += 4
			tpB := payload[pos : pos+tpL]; pos += tpL
			ptL := int(binary.BigEndian.Uint32(payload[pos : pos+4])); pos += 4
			ptB := payload[pos : pos+ptL]; pos += ptL
			fgL := int(binary.BigEndian.Uint32(payload[pos : pos+4])); pos += 4
			fgB := payload[pos : pos+fgL]; pos += fgL
			bgL := int(binary.BigEndian.Uint32(payload[pos : pos+4])); pos += 4
			bgB := payload[pos : pos+bgL]; pos += bgL

			p := make([]uint8, w*h)
			szR, tpR, ptR := &bitReader{data: szB}, &bitReader{data: tpB}, &bitReader{data: ptB}
			fgIdx, bgIdx := 0, 0
			for my := 0; my < (h/mb)*mb; my += mb {
				for mx := 0; mx < (w/mb)*mb; mx += mb {
					isBig, _ := szR.readBit()
					if isBig {
						fg, bg := fgB[fgIdx], bgB[bgIdx]; fgIdx++; bgIdx++
						tpR.readBit()
						for yy := 0; yy < mb; yy++ {
							for xx := 0; xx < mb; xx++ {
								bit, _ := ptR.readBit()
								val := bg; if bit { val = fg }; p[(my+yy)*w+(mx+xx)] = val
							}
						}
					} else {
						for by := 0; by < mb; by += sb {
							for bx := 0; bx < mb; bx += sb {
								fg, bg := fgB[fgIdx], bgB[bgIdx]; fgIdx++; bgIdx++
								isPt, _ := tpR.readBit()
								for yy := 0; yy < sb; yy++ {
									for xx := 0; xx < sb; xx++ {
										val := fg; if isPt { bit, _ := ptR.readBit(); val = bg; if bit { val = fg } }
										p[(my+by+yy)*w+(mx+bx+xx)] = val
									}
								}
							}
						}
					}
				}
			}
			planes[i] = p
		} else {
			planes[i] = make([]uint8, w*h)
			for j := range planes[i] { planes[i][j] = 128 }
		}
	}

	for y := 0; y < h; y++ {
		for x := 0; x < w; x++ {
			idx := y*w + x
			Y, Cb, Cr := int32(planes[0][idx]), int32(planes[1][idx])-128, int32(planes[2][idx])-128
			R := Y + ((91881 * Cr) >> 16)
			G := Y - ((22554*Cb + 46802*Cr) >> 16)
			B := Y + ((116130 * Cb) >> 16)
			cl := func(v int32) uint8 { if v < 0 { return 0 }; if v > 255 { return 255 }; return uint8(v) }
			dst.SetRGBA(x, y, color.RGBA{cl(R), cl(G), cl(B), 255})
		}
	}
	return dst, nil
}

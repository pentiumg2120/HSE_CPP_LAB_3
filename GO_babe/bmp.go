package main

import (
	"encoding/binary"
	"fmt"
	"image"
	"image/color"
	"io"
	"os"
)

func LoadBMP(path string) (image.Image, error) {
	f, err := os.Open(path); if err != nil { return nil, err }; defer f.Close()
	h := make([]byte, 14); if _, err := io.ReadFull(f, h); err != nil { return nil, err }
	if h[0] != 'B' || h[1] != 'M' { return nil, fmt.Errorf("not BMP") }
	off := binary.LittleEndian.Uint32(h[10:14])
	ihsB := make([]byte, 4); if _, err := io.ReadFull(f, ihsB); err != nil { return nil, err }
	ihs := binary.LittleEndian.Uint32(ihsB)
	ih := make([]byte, ihs-4); if _, err := io.ReadFull(f, ih); err != nil { return nil, err }
	w, hgt := int32(binary.LittleEndian.Uint32(ih[0:4])), int32(binary.LittleEndian.Uint32(ih[4:8]))
	if binary.LittleEndian.Uint16(ih[10:12]) != 32 { return nil, fmt.Errorf("only 32-bit") }
	f.Seek(int64(off), 0)
	absH := int(hgt); if absH < 0 { absH = -absH }
	img := image.NewRGBA(image.Rect(0, 0, int(w), absH))
	row := make([]byte, int(w)*4)
	for y := 0; y < absH; y++ {
		io.ReadFull(f, row)
		tY := y; if hgt > 0 { tY = absH - 1 - y }
		for x := 0; x < int(w); x++ {
			img.SetRGBA(x, tY, color.RGBA{row[x*4+2], row[x*4+1], row[x*4], 255})
		}
	}
	return img, nil
}

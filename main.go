package main

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Fprint(os.Stderr, "Usage: babe <input.bmp> [quality] [bw] OR babe <input.babe> -t\n")
		os.Exit(1)
	}
	inP := os.Args[1]
	ext := strings.ToLower(filepath.Ext(inP))
	if ext == ".babe" {
		if len(os.Args) > 2 && os.Args[2] == "-t" {
			f, _ := os.Open(inP); defer f.Close(); d, _ := io.ReadAll(f)
			img, _ := Decode(d, false); renderToTerminal(img)
		}
		return
	}
	q := 70
	if len(os.Args) >= 3 { if v, err := strconv.Atoi(os.Args[2]); err == nil { q = v } }
	bw := false
	for _, a := range os.Args[2:] { if a == "bw" { bw = true } }
	img, _ := LoadBMP(inP)
	start := time.Now(); enc, _ := Encode(img, q, bw); dur := time.Since(start)
	outP := strings.TrimSuffix(inP, ext) + ".babe"
	f, _ := os.Create(outP); defer f.Close(); f.Write(enc)
	fmt.Printf("%s -> %s (q=%d, time=%s)\n", inP, outP, q, dur)
}

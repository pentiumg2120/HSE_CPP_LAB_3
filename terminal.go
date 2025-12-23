package main

import (
	"fmt"
	"image"
	"os"

	"golang.org/x/term"
)

func renderToTerminal(img image.Image) {
	bounds := img.Bounds()
	width, height := bounds.Dx(), bounds.Dy()
	termW, termH, err := term.GetSize(int(os.Stdout.Fd()))
	if err != nil { termW, termH = 80, 24 }

	scale := float64(termW) / float64(width)
	if sH := float64(termH*2) / float64(height); sH < scale { scale = sH }
	if scale > 1.0 { scale = 1.0 }

	newW, newH := int(float64(width)*scale), int(float64(height)*scale)
	if newH%2 != 0 { newH-- }

	for y := 0; y < newH; y += 2 {
		for x := 0; x < newW; x++ {
			c1 := img.At(bounds.Min.X+int(float64(x)/scale), bounds.Min.Y+int(float64(y)/scale))
			c2 := img.At(bounds.Min.X+int(float64(x)/scale), bounds.Min.Y+int(float64(y+1)/scale))
			r1, g1, b1, _ := c1.RGBA()
			r2, g2, b2, _ := c2.RGBA()
			fmt.Printf("\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm▀", r1>>8, g1>>8, b1>>8, r2>>8, g2>>8, b2>>8)
		}
		fmt.Print("\x1b[0m\n")
	}
}

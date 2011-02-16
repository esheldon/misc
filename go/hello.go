/* vim: set filetype=go : */
package main
import (
	"fmt"
	"os"
	"scanner"
)


func main() {
	type mystruct struct {
		x int
		y int
		s string
	}

	f8 := make([]float64, 6)

	f8[3] = 25


	//tmp := make([]byte, 5)

	//tmp := bytes.Buffer()

	tmps := "11111"
	fname := "test.dat"

	fobj,err := os.Open("test.dat",0,0)
	if fobj==nil {
		fmt.Printf("Could not open file %s: error %s\n",fname,err)
	}

	//fobj.Read(tmp)

	sc := scanner.NewScanner(fobj)

	//buf := bytes.NewBuffer(tmp)
	//tmps = buf.String()

	// doesn't work
	//b := syscall.StringByteSlice(tmps)
	//fobj.Read(b[0:len(b)-1])

	tmps = sc.NextString()

	msvec := make([]mystruct, 10)
	msvec[0].x = 3
	msvec[0].y = 5
	msvec[0].s = "test string field"

	fmt.Println(msvec)
	//fmt.Printf("%s\n", tmp)
	fmt.Println(tmps)
}

Before compilation we should install leptonica, tesseract and opencv libraries

leptonica, tesseract installation process

first of all we need to install following libraries
```
    sudo apt-get install libopencv-dev
    sudo apt-get install g++
    sudo apt-get install autoconf automake libtool
    sudo apt-get install autoconf-archive
    sudo apt-get install pkg-config
    sudo apt-get install libpng12-dev
    sudo apt-get install libjpeg8-dev
    sudo apt-get install libtiff5-dev
    sudo apt-get install zlib1g-dev
```
training tools
```
    sudo apt-get install libicu-dev
    sudo apt-get install libpango1.0-dev
    sudo apt-get install libcairo2-dev
 ```

leptonica package skould be installed before tesseract
```
    sudo apt-get install libleptonica-dev
```

tesseract
```
    sudo apt-get install tesseract-ocr
    sudo apt-get install libtesseract-dev
    sudo apt-get install tesseract-ocr-all
```

/! To be sure that everything is OK  check packages
    ```pkg-config --cflags tesseract```

This command output should be something like below maintained text
    `-I/usr/include/tesseract -I/usr/include/leptonica`

install zxing library for barcode extraction
just open the link and follow "Building using CMake" part
    ```https://github.com/glassechidna/zxing-cpp```
 
Afther that we can make the sources and run the executable
 
Compile
```
    make
```
tesserocr
Requires libtesseract (>=3.04) and libleptonica (>=1.71).
```
    pip install tesserocr
```


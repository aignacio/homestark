#Para compilar para o TARGET=z1 se necessita do msp430-gcc 4.7
#para isso:
tar -vzxf msp430-47.tar.gz /usr/bin
cd /usr/bin
ln -s msp430-47/bin/msp430* . #Cria link simbólico para o toolchain msp430-*



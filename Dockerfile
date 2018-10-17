FROM base/archlinux
MAINTAINER Ahmed Abdel Aal <eng.ahmedhussein89@gmail.com>

RUN pacman -Syu --noconfirm

RUN pacman -S --noconfirm \
                gcc       \
                cmake     \
                glew      \
                freeglut  \
                soil      \
                glm       \
                git       \
                make

RUN mkdir /src
WORKDIR /src
RUN git clone https://github.com/wow2006/OpenGLBuildHighPerformanceGraphics.git
WORKDIR /src/build
RUN cmake ../OpenGLBuildHighPerformanceGraphics
CMD make

# openmediaplayer

```bash

sudo apt install -y qtbase5-dev \
    ccache \
    cmake \
    libx11-dev \
    zlib1g-dev \
    libhildon-thumbnail-dev \
    libgdk-pixbuf2.0-dev \
    libhildon1-dev \
    libdbus-c++-dev \
    libplayback-1-dev \
    libplayback-1-dbg \
    libpng-dev \
    qtbase5-dev \
    libqt5svg5-dev \
    libqt5maemo5-dev \
    libqt5x11extras5-dev \
    qtquickcontrols2-5-dev \
    qtdeclarative5-dev \
    qtdeclarative5-dev-tools
```

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -Bbuild . && make -Cbuild -j2
```
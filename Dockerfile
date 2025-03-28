FROM ghcr.io/wiiu-env/devkitppc:20241128

COPY --from=ghcr.io/wiiu-env/wiiumodulesystem:20250219 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20250208 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20250204 /artifacts $DEVKITPRO

RUN \
mkdir wut && \
cd wut && \
git init . && \
git remote add origin https://github.com/deReeperJosh/wut.git && \
git fetch --depth 1 origin d389bb66b6cc406ba681289b7a8db572a51395d3 && \
git checkout FETCH_HEAD
WORKDIR /wut
RUN make -j$(nproc)
RUN make install

WORKDIR /project

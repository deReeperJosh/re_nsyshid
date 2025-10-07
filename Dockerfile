FROM ghcr.io/wiiu-env/devkitppc:latest

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:latest /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:latest /artifacts $DEVKITPRO

WORKDIR project
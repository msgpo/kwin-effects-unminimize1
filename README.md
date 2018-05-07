![Slow motion](demo/slow-motion.gif)

"Unminimize 1" is a KWin effect that animates the unminimizing of windows.

## Installation

### Arch Linux

For Arch Linux [kwin-effects-unminimize1](https://aur.archlinux.org/packages/kwin-effects-unminimize1/)
is available in the AUR.

### Fedora

```sh
sudo dnf copr enable zzag/kwin-effects
sudo dnf refresh
sudo dnf install kwin-effects-unminimize1
```

### Ubuntu

```sh
sudo add-apt-repository ppa:vladzzag/kwin-effects
sudo apt install libkwin4-effect-unminimize1
```

### From source

```sh
git clone https://github.com/zzag/kwin-effects-unminimize1.git
cd kwin-effects-unminimize1
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make -jN
sudo make install
```

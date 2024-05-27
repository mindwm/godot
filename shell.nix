{ pkgs ? import <nixpkgs> {}, unstable ? import <unstable> {} }:
with pkgs;
let
  inherit (pkgs);
  libs = [ xorg.libX11 xorg.libXcursor xorg.libXinerama xorg.libXrandr xorg.libXrender libpulseaudio xorg.libXi xorg.libXext xorg.libXfixes freetype openssl alsaLib libGLU zlib yasm udev ];
in
pkgs.stdenv.mkDerivation {
  name = "Godot";
  nativeBuildInputs = [ pkgconfig scons ];
  buildInputs = libs;
  shellHook = ''
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${lib.makeLibraryPath libs}"
    echo "Godot nix dev environment"
  '';
}

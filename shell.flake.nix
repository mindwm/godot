{ pkgs ? builtins.currentSystem }:
with pkgs;
let
  inherit (pkgs);
  libs = [ scons pkgconfig xorg.libX11 xorg.libXcursor xorg.libXinerama xorg.libXrandr xorg.libXrender libpulseaudio xorg.libXi xorg.libXext xorg.libXfixes freetype openssl alsaLib libGLU zlib yasm
  vulkan-loader
  libGL
  
  ];
in
pkgs.stdenv.mkDerivation {
  name = "Godot";
  buildInputs = [ pkgconfig ] ++ libs;
  shellHook = ''
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${lib.makeLibraryPath libs}"
    echo "Godot nix dev environment"
  '';
}

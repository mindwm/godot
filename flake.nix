{
  description = "Dev envrinment for Godot";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem
      (system:
         let mkPkgs = pkgs: extraOverlays: import pkgs {
               inherit system;
               config.allowUnfree = true; # forgive me Stallman senpai
             };
             pkgs = mkPkgs nixpkgs [];
        in
        {
          devShell = import ./shell.flake.nix { inherit pkgs; };
        }
      );
}

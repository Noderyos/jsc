{
    description = "A very basic flake";
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    };

    outputs = { self, nixpkgs }: 
      let
          system = "x86_64-linux";
          pkgs = import nixpkgs { inherit system; };
      in {
          devShells.${system}.default = pkgs.mkShell {
            buildInputs = [
              pkgs.gnumake pkgs.python3 pkgs.readline pkgs.ccls pkgs.gdb pkgs.gf
            ];
          };
          packages.${system}.default = pkgs.stdenv.mkDerivation {
            name = "jsc";
            version = "0.1";

            src = ./.;
          };
      };
}

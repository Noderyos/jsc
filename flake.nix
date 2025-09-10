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
          packages.${system}.default =
            let
              unicodeData = pkgs.fetchurl {
                url = "https://www.unicode.org/Public/16.0.0/ucd/UnicodeData.txt";
                hash = "sha256-/1jlgjvQlRZlZKAG5H0RETCBPc+L8jTvefpRqHDttI8=";
              };
              specialCasing = pkgs.fetchurl {
                url = "https://www.unicode.org/Public/16.0.0/ucd/SpecialCasing.txt";
                hash = "sha256-jV3jVO73nyOVpUycfc67rz0w/JYtD4VhHql6qXOgxFE=";
              };
            in pkgs.stdenv.mkDerivation {
              name = "jsc";
              version = "0.2";

              enableParallelBuilding = true;

              buildInputs = [ pkgs.curl pkgs.python3 ];

              preBuild = ''
                cp ${unicodeData} mujs/UnicodeData.txt
                cp ${specialCasing} mujs/SpecialCasing.txt
              '';

              src = pkgs.fetchFromGitHub {
                owner = "Noderyos";
                repo = "jsc";
                rev = "14b75ff56ebe2f9a43c1344e7eeb9c3c56944faa";
                fetchSubmodules = true;
                hash = "sha256-E2SVH4wCnoezE4vqOfbVN8dXLcNk7IJmF8lvLPcw/os=";
              };

              makeFlags = ["jsc"];

              installPhase = ''
                mkdir -p $out/bin
                cp jsc $out/bin
              '';
            };
      };
}

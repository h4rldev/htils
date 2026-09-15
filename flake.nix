{
  description = "The flake for the htils library.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    conjure.url = "git+https://codeberg.org/h4rl/conjure.git";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    conjure,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};
      pversion = "0.1.0";

      nativeInputs = [
        conjure.packages.${system}.default
        pkgs.gcc
        pkgs.mold
        pkgs.doxygen
      ];

      mkHtils = {
        name,
        profile,
        artifact,
        pc,
      }: let
        artifactStem =
          pkgs.lib.removePrefix "lib"
          (pkgs.lib.removeSuffix ".so"
            (pkgs.lib.removeSuffix ".a" (baseNameOf artifact)));
      in
        pkgs.stdenv.mkDerivation {
          pname = name;
          version = pversion;

          src = ./.;

          nativeBuildInputs = nativeInputs;
          outputs = ["out" "doc"];

          buildPhase = ''
            runHook preBuild
            conjure as ${profile} build
            doxygen
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            mkdir -p $out/lib/pkgconfig
            mkdir -p $out/include/htils/
            mkdir -p $doc/share/doc/htils/

            cp ${artifact} $out/lib
            sed -e "s|^prefix=.*|prefix=$out|" \
              -e "s|^libdir=.*|libdir=$out/lib|" \
              lib/${profile}/pkgconfig/${artifactStem}.pc \
              > $out/lib/pkgconfig/${pc}

            cp include/htils.h $out/include
            cp include/htils/* $out/include/htils/
            cp -r doc/html/* $doc/share/doc/htils/

            runHook postInstall
          '';
        };
    in {
      packages = {
        htils = mkHtils {
          name = "htils";
          profile = "htils-release";
          artifact = "lib/htils-release/libhtils.so";
          pc = "htils.pc";
        };

        htils-static = mkHtils {
          name = "htils-static";
          profile = "htils-release-static";
          artifact = "lib/htils-release-static/libhtils.a";
          pc = "htils-static.pc";
        };

        htils-debug = mkHtils {
          name = "htils-debug";
          profile = "htils-debug";
          artifact = "lib/htils-debug/libhtils-debug.a";
          pc = "htils-debug.pc";
        };

        htils-threadsafe = mkHtils {
          name = "htils-threadsafe";
          profile = "htils-release-threadsafe";
          artifact = "lib/htils-release-threadsafe/libhtils.so";
          pc = "htils-threadsafe.pc";
        };

        htils-debug-threadsafe = mkHtils {
          name = "htils-debug-threadsafe";
          profile = "htils-debug-threadsafe";
          artifact = "lib/htils-debug-threadsafe/libhtils-debug.a";
          pc = "htils-debug-threadsafe.pc";
        };

        htils-threadsafe-static = mkHtils {
          name = "htils-threadsafe-static";
          profile = "htils-release-static-threadsafe";
          artifact = "lib/htils-release-static-threadsafe/libhtils.a";
          pc = "htils-threadsafe-static.pc";
        };
      };

      devShells.default = pkgs.mkShell {
        name = "htils";
        description = "A small set of utilities that i use very often.";

        buildInputs = with pkgs; [
          glibc
          glibc.static
          mold
        ];

        packages = with pkgs; [
          bear
          doxygen
          bun
          conjure.packages.${system}.default

          clang-tools
          nixd
          prettierd
          ## doctoc
        ];
      };
    });
}

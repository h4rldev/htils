{
  description = "The flake for the htils library.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      packages.default = pkgs.stdenv.mkDerivation {
        pname = "htils";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [just gcc mold doxygen];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just release

          doxygen
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/lib
          mkdir -p $out/include/htils/
          mkdir -p $doc/share/doc/htils/

          cp lib/libhtils.so $out/lib
          cp include/htils.h $out/include
          cp include/htils/* $out/include/htils/
          cp -r doc/html/* $doc/share/doc/htils/

          runHook postInstall
        '';
      };

      packages.static = pkgs.stdenv.mkDerivation {
        pname = "htils-static";
        version = "0.1.0";

        src = ./.;

        outputs = ["out" "doc"];

        nativeBuildInputs = with pkgs; [just gcc mold glibc.static doxygen];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just release static

          doxygen
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/lib
          mkdir -p $out/include/htils/
          mkdir -p $doc/share/doc/htils/

          cp lib/libhtils.a $out/lib
          cp include/htils.h $out/include
          cp include/htils/* $out/include/htils/
          cp -r doc/html/* $doc/share/doc/htils/

          runHook postInstall
        '';
      };

      packages.debug = pkgs.stdenv.mkDerivation {
        pname = "htils-debug";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [just gcc mold doxygen];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just debug
          doxygen
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/lib
          mkdir -p $out/include/htils/
          mkdir -p $doc/share/doc/htils

          cp lib/libhtils-debug.a $out/lib
          cp include/htils.h $out/include
          cp include/htils/* $out/include/htils/
          cp -r doc/html/* $doc/share/doc/htils/

          runHook postInstall
        '';
      };

      devShells.default = pkgs.mkShell {
        name = "htils";
        description = "A small set of utilities that i use very often.";

        buildInputs = with pkgs; [
          h2o
          glibc
          glibc.static
          mold
        ];

        packages = with pkgs; [
          bear
          just
          doxygen

          clang-tools
          nixd
          prettierd
          doctoc
        ];
      };
    });
}

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
      packages.h2otils = pkgs.stdenv.mkDerivation {
        pname = "h2otils";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.just
          pkgs.gcc
          pkgs.mold
          pkgs.doxygen
          pkgs.h2o
          self.packages.${system}.htils
        ];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just h2otils-release

          doxygen
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/lib
          mkdir -p $out/include/h2otils/
          mkdir -p $doc/share/doc/h2otils/

          cp lib/libh2otils.so $out/lib
          cp include/h2otils.h $out/include
          cp -r include/h2otils/* $out/include/h2otils/
          cp -r doc/html/* $doc/share/doc/h2otils/

          runHook postInstall
        '';
      };

      packages.h2otils-static = pkgs.stdenv.mkDerivation {
        pname = "h2otils-static";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [just gcc mold glibc.static doxygen h2o];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just h2otils-release static

          doxygen
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/lib
          mkdir -p $out/include/h2otils/
          mkdir -p $doc/share/doc/h2otils/

          cp lib/libh2otils.a $out/lib
          cp include/h2otils.h $out/include
          cp -r include/h2otils/* $out/include/h2otils/
          cp -r doc/html/* $doc/share/doc/h2otils/

          runHook postInstall
        '';
      };

      packages.h2otils-debug = pkgs.stdenv.mkDerivation {
        pname = "h2otils-debug";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = [
          pkgs.just
          pkgs.gcc
          pkgs.mold
          pkgs.doxygen
          pkgs.h2o
          self.packages.${system}.htils-debug
        ];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just h2otils-debug

          doxygen
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/lib
          mkdir -p $out/include/h2otils/
          mkdir -p $doc/share/doc/h2otils/

          cp lib/libh2otils-debug.a $out/lib
          cp include/h2otils.h $out/include
          cp -r include/h2otils/* $out/include/h2otils/
          cp -r doc/html/* $doc/share/doc/h2otils/

          runHook postInstall
        '';
      };

      packages.htils = pkgs.stdenv.mkDerivation {
        pname = "htils";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [just gcc mold doxygen];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just htils-release

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

      packages.htils-static = pkgs.stdenv.mkDerivation {
        pname = "htils-static";
        version = "0.1.0";

        src = ./.;

        outputs = ["out" "doc"];

        nativeBuildInputs = with pkgs; [just gcc mold glibc.static doxygen];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just htils-release-static

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

      packages.htils-debug = pkgs.stdenv.mkDerivation {
        pname = "htils-debug";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [just gcc mold doxygen];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just htils-debug
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

      packages.htils-threadsafe = pkgs.stdenv.mkDerivation {
        pname = "htils-threadsafe";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [just gcc mold doxygen];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just htils-release-threadsafe
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

      packages.htils-debug-threadsafe = pkgs.stdenv.mkDerivation {
        pname = "htils-debug-threadsafe";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [just gcc mold doxygen];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just htils-debug-threadsafe
          doxygen
          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall
          mkdir -p $out/lib
          mkdir -p $out/include/htils/
          mkdir -p $doc/share/doc/htils/

          cp lib/libhtils-debug.a $out/lib
          cp include/htils.h $out/include
          cp include/htils/* $out/include/htils/
          cp -r doc/html/* $doc/share/doc/htils/

          runHook postInstall
        '';
      };

      packages.htils-threadsafe-static = pkgs.stdenv.mkDerivation {
        pname = "htils-threadsafe-static";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [just gcc mold doxygen];

        outputs = ["out" "doc"];

        buildPhase = ''
          runHook preBuild
          sed -i 's|#!/usr/bin/env bash|#!${pkgs.bash}/bin/bash|' justfile
          just htils-release-threadsafe-static
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
          bun

          clang-tools
          nixd
          prettierd
          doctoc
        ];
      };
    });
}

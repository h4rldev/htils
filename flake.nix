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
      packages.default = pkgs.hello;

      devShells.default = pkgs.mkShell {
        name = "htils";
        description = "A small set of utilities that i use very often.";

        buildInputs = with pkgs; [
          h2o
          mold
        ];

        packages = with pkgs; [
          bear
          just

          clang-tools
          nixd
          prettierd
          doctoc
        ];
      };
    });
}

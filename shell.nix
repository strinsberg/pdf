{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {

  nativeBuildInputs = with pkgs; [
    cmake pkg-config clang clang-tools
  ];

  buildInputs = with pkgs; [
    zlib gtest
  ];

  shellHook = ''
    export CC=clang
    export CXX=clang++
  '';
}


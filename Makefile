include wg21/Makefile

all: forward_like
forward_like: forward_like.cpp
	clang++ -std=c++20 $< -o $@
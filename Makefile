include wg21/Makefile

all: forward_like_lang forward_like_merge forward_like_tuple forward_like

forward_like_lang: forward_like_lang.cpp forward_like.hpp meta.hpp
	clang++ -std=c++20 $< -o $@

forward_like_tuple: forward_like_tuple.cpp forward_like.hpp meta.hpp
	clang++ -std=c++20 $< -o $@

forward_like_merge: forward_like_merge.cpp forward_like.hpp meta.hpp
	clang++ -std=c++20 $< -o $@

forward_like: forward_like.cpp forward_like.hpp meta.hpp
	clang++ -std=c++20 $< -o $@

clean-cpp:
	rm -f forward_like forward_like_merge forward_like_tuple forward_like_lang
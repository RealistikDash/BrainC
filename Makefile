#!/usr/bin/env -S make -f
brain: brain.c
	gcc $^ -o $@ -O3

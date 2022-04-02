summit:
	mpic++ Writer.cpp -o writer $(shell adios2-config --cxx)

crusher:
	cc Writer.cpp -o writer $(shell adios2-config --cxx) /opt/cray/pe/gcc/11.2.0/snos/lib64/libstdc++.so

d:
	rm -rf *.sst job_* core.* *.erf *.job

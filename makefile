all:
	mpic++ Writer.cpp -o writer $(shell adios2-config --cxx)

d:
	rm -rf *.sst job_* core.* *.erf *.job

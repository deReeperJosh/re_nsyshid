
workdir :                       = $(pwd)
                  mkfile_path : = $(abspath $(lastword $(MAKEFILE_LIST)) /..)

                                                  build /
                                          http.o : $(mkfile_path) / http.cpp $(mkfile_path) / http.hpp $(CXX) $(CXXFLAGS) -
                                  c $(mkfile_path) / http.cpp - o build / http.o

                                                                                  build /
                                                                        websock.o : $(mkfile_path) / websock.cpp $(mkfile_path) / http.hpp $(CXX) $(CXXFLAGS) -
                                  c $(mkfile_path) / websock.cpp - o build / websock.o

# location of the Python header files
 
PYTHON_VERSION = 2.7
PYTHON_INCLUDE = /usr/include/python$(PYTHON_VERSION)
 
# location of the Boost Python include files and library
 
BOOST_INC = /usr/include
BOOST_LIB = /usr/lib

GAMBATTE_INC = /home/aeb/Documents/zelda-ai/custom_gambatte/pygambatte/gambatte/libgambatte/include
GAMBATTE_COMMON = /home/aeb/Documents/zelda-ai/custom_gambatte/pygambatte/gambatte/common
GAMBATTE_LIB = /home/aeb/Documents/zelda-ai/custom_gambatte/pygambatte/

# compile mesh classes
TARGET = pygambatte

libgambatte.a:
	cd gambatte/libgambatte && scons && cp libgambatte.a ../..

$(TARGET).so: $(TARGET).o
	g++ -std=c++11 -o $(TARGET).so -shared -Wl,--export-dynamic $(TARGET).o -L$(BOOST_LIB) -L$(GAMBATTE_LIB) -lboost_python -L/usr/lib/python$(PYTHON_VERSION)/config -lpython2.7 -lgambatte 
 
$(TARGET).o: $(TARGET).cpp libgambatte.a
	g++ -std=c++11 -I$(PYTHON_INCLUDE) -I$(BOOST_INC) -I$(GAMBATTE_INC) -I$(GAMBATTE_COMMON) -fPIC -fpermissive -c $(TARGET).cpp

clean:
	rm -f $(TARGET).o $(TARGET).so

realclean:
	rm -f $(TARGET).o $(TARGET).so
	rm -f libgambatte.a
	cd gambatte/libgambatte && rm libgambatte.a libgambatte.so
	cd gambatte/libgambatte/src && rm *.o */*.o

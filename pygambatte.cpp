#include <boost/python.hpp>
#include <numpy/ndarrayobject.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <sys/stat.h>
#include <cerrno>
#include <stdexcept>
#include <string>

#include "gambatte.h"
#include "inputgetter.h"

#define GB_SAMPLES_PER_FRAME 35112
#define GB_SAMPLE_OVERFLOW 2064
#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

off_t get_file_length(std::string const& filename)
{
    struct stat st;
    if (stat(filename.c_str(), &st) == -1)
        throw std::runtime_error(std::strerror(errno));
    return st.st_size;
}

void debug_read_handler(unsigned clazz, unsigned offset, uint8_t value, bool exec)
{
	return;
}

void debug_write_handler(unsigned clazz, unsigned offset, uint8_t value)
{
	return;
}

void debug_trace_handler(uint16_t _pc)
{
	return;
}

boost::python::object get_video_numpy_array(std::vector<gambatte::uint_least32_t> const& vec)
{
	npy_intp size = vec.size();
	npy_intp dims[] = {GB_SCREEN_HEIGHT, GB_SCREEN_WIDTH};

	uint32_t * data = (uint32_t *) (&vec[0]);

	PyObject * pyObj = PyArray_SimpleNewFromData(2, dims, NPY_UINT32, data);

	boost::python::handle<> handle(pyObj);
	boost::python::numeric::array arr(handle);

	return arr.copy();
}

boost::python::object get_video_numpy_array2(std::vector<gambatte::uint_least32_t> const& vec)
{
	npy_intp size = vec.size();
	npy_intp dims[] = {GB_SCREEN_HEIGHT, GB_SCREEN_WIDTH, 4};

	uint8_t * data = (uint8_t *) (&vec[0]);

	PyObject * pyObj = PyArray_SimpleNewFromData(3, dims, NPY_UINT8, data);

	boost::python::handle<> handle(pyObj);
	boost::python::numeric::array arr(handle);



	return arr.copy();
}

boost::python::object stdVecToNumpyArray(std::vector<gambatte::uint_least32_t> const& vec)
{
	npy_intp size = vec.size();

	gambatte::uint_least32_t * data = size ? const_cast<gambatte::uint_least32_t *>(&vec[0])
	                                       : static_cast<gambatte::uint_least32_t *>(NULL); 

	PyObject * pyObj = PyArray_SimpleNewFromData(1, &size, NPY_UINT32, data);

	boost::python::handle<> handle(pyObj);
	boost::python::numeric::array arr(handle);

	return arr.copy();	
}

class GetInput : public gambatte::InputGetter {
public:
	unsigned is;

	GetInput() : is(0) {};
	virtual unsigned operator()() { return is; }
};

class PyGambatteGB {
public:
	gambatte::GB* gb;
	GetInput inputGetter;
	std::vector<gambatte::uint_least32_t> videoBuf;
	std::vector<gambatte::uint_least32_t> soundBuf;
	gambatte::uint_least32_t *vidptr, *sndptr;
	gambatte::debugbuffer dbg;

	std::ptrdiff_t pitch = GB_SCREEN_WIDTH;

	PyGambatteGB() : videoBuf(GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT), soundBuf(GB_SAMPLES_PER_FRAME + GB_SAMPLE_OVERFLOW)
	{
		gb = new gambatte::GB;

		gb->setInputGetter(&inputGetter);

		uint8_t* tmp = new uint8_t[98816];
		memset(tmp, 0, 98816);
		
		dbg.wram = tmp;
		dbg.bus = tmp + 32768;
		dbg.ioamhram = tmp + 98304;
		dbg.read = debug_read_handler;
		dbg.write = debug_write_handler;
		dbg.trace = debug_trace_handler;
		dbg.trace_cpu = false;

		gb->set_debug_buffer(dbg);

		vidptr = &videoBuf[0];
		sndptr = &soundBuf[0];
	}

	~PyGambatteGB()
	{
		delete gb;
	}

	gambatte::LoadRes load(std::string const &romfile, unsigned flags = 0)
	{
		gambatte::LoadRes res;

		res = gb->load(romfile, flags);

		size_t sramsize = gb->getSaveRam().second;
		size_t romsize = get_file_length(romfile);

		dbg.sram = new uint8_t[(sramsize + 4095) >> 12 << 12];
		dbg.cart = new uint8_t[(romsize + 4095) >> 12 << 12];

		return res;
	}
	
	boost::python::tuple runFor(unsigned samples)
	{
		unsigned _samples = samples;
		int retval = gb->runFor(vidptr, pitch, sndptr, _samples);
		return boost::python::make_tuple(retval, _samples);
	}

	void run_frame()
	{
		signed result = -1;
		unsigned samples = GB_SAMPLES_PER_FRAME;

		while (result < 0)
		{
			result = gb->runFor(vidptr, pitch, sndptr, samples);
			samples = GB_SAMPLES_PER_FRAME;
		}
	}

	void set_input(unsigned input)
	{
		inputGetter.is = input;
	}

	boost::python::object get_video()
	{
		return get_video_numpy_array(videoBuf);
	}

	boost::python::object get_video2()
	{
		return get_video_numpy_array2(videoBuf);
	}

	void reset()
	{
		gb->reset();
	}

	void print_interrupt_timing (void)
	{
		gb->print_interrupt_timing();
	}

	unsigned get_cycle_counter(void)
	{
		return gb->get_cycle_counter();
	}

	boost::python::object getWorkRam()
	{
		char * wram = std::get<0>(gb->getWorkRam());
		size_t size = std::get<1>(gb->getWorkRam());

		PyObject * pyObj = PyArray_SimpleNewFromData(1, &size, NPY_UINT8, wram);

		boost::python::handle<> handle(pyObj);
		boost::python::numeric::array arr(handle);

		return arr.copy();
	}

	boost::python::object getSaveRam()
	{
		char * sram = std::get<0>(gb->getSaveRam());
		size_t size = std::get<1>(gb->getSaveRam());

		PyObject * pyObj = PyArray_SimpleNewFromData(1, &size, NPY_UINT8, sram);

		boost::python::handle<> handle(pyObj);
		boost::python::numeric::array arr(handle);

		return arr.copy();
	}

	boost::python::object getVideoRam()
	{
		char * vram = std::get<0>(gb->getVideoRam());
		size_t size = std::get<1>(gb->getVideoRam());

		PyObject * pyObj = PyArray_SimpleNewFromData(1, &size, NPY_UINT8, vram);

		boost::python::handle<> handle(pyObj);
		boost::python::numeric::array arr(handle);

		return arr.copy();
	}	

	boost::python::object getIoRam()
	{
		char * ioram = std::get<0>(gb->getIoRam());
		size_t size = std::get<1>(gb->getIoRam());

		PyObject * pyObj = PyArray_SimpleNewFromData(1, &size, NPY_UINT8, ioram);

		boost::python::handle<> handle(pyObj);
		boost::python::numeric::array arr(handle);

		return arr.copy();
	}

	uint32_t getRegister(gambatte::GB::cpu_register reg) {
		return gb->get_cpureg(reg);
	}

	void setRegister(gambatte::GB::cpu_register reg, uint32_t val) {
		gb->set_cpureg(reg, val);
	}

//	boost::python::object saveStateToFile(boost::python::object filepath)
//	{
//		std::string path = boost::python::extract<std::string>(filepath);
//		gb->saveState
//	}

	boost::python::object getSaveState()
	{
		std::vector<char> data;
		//data.reserve(65536*64);
		gb->saveState(data);

		size_t size = data.size();

		PyObject * pyObj = PyArray_SimpleNewFromData(1, &size, NPY_UINT8, &data[0]);

		boost::python::handle<> handle(pyObj);
		boost::python::numeric::array arr(handle);

		return arr.copy();		
	}

//	void loadSaveState(boost::python::numeric::array& state)
//	{
//		boost::python::tuple shape = boost::python::extract<boost::python::tuple>(state.getshape());
//		int len = boost::python::extract<int>(shape[0]);
//
//		std::vector<char> vdata(len, 0);
//		char * data_ptr = vdata.data();
//
//		for (int i = 0; i < len; i++) {
//			vdata[i] = *(data_ptr + i);
//		}
//
//		gb->loadState(vdata);
//	}

	boost::python::object loadSaveState(PyObject* state) {
		boost::python::tuple shape = boost::python::extract<boost::python::tuple>(PyObject_GetAttrString(state, "shape"));
		long int len = boost::python::extract<int>(shape[0]);

		std::vector<char> vdata(len, 0);
		char * data_ptr = ((PyArrayObject*) state)->data;

		// It's copying to itself!
		for (long int i = 0; i < len; i++) {
			vdata[i] = *(data_ptr + i);
		}

		PyObject * pyObj = PyArray_SimpleNewFromData(1, &len, NPY_UINT8, &vdata[0]);

		boost::python::handle<> handle(pyObj);
		boost::python::numeric::array arr(handle);

		return arr.copy();	

		// gb->loadState(vdata);
	}



};

BOOST_PYTHON_MODULE(pygambatte)
{
	using namespace boost::python;
	//def("greet", greet);

	boost::python::numeric::array::set_module_and_type("numpy", "ndarray");
    import_array();

	enum_<gambatte::LoadRes>("LoadRes")
		.value("LOADRES_BAD_FILE_OR_UNKNOWN_MBC", gambatte::LoadRes::LOADRES_BAD_FILE_OR_UNKNOWN_MBC)
		.value("LOADRES_IO_ERROR", gambatte::LoadRes::LOADRES_IO_ERROR)
		.value("LOADRES_UNSUPPORTED_MBC_HUC3", gambatte::LoadRes::LOADRES_UNSUPPORTED_MBC_HUC3)
		.value("LOADRES_UNSUPPORTED_MBC_TAMA5", gambatte::LoadRes::LOADRES_UNSUPPORTED_MBC_TAMA5)
		.value("LOADRES_UNSUPPORTED_MBC_POCKET_CAMERA", gambatte::LoadRes::LOADRES_UNSUPPORTED_MBC_POCKET_CAMERA)
		.value("LOADRES_UNSUPPORTED_MBC_MBC4", gambatte::LoadRes::LOADRES_UNSUPPORTED_MBC_MBC4)
		.value("LOADRES_UNSUPPORTED_MBC_MMM01", gambatte::LoadRes::LOADRES_UNSUPPORTED_MBC_MMM01)
		.value("LOADRES_OK", gambatte::LoadRes::LOADRES_OK)
	;

	enum_<gambatte::InputGetter::Button>("Button")
		.value("A", gambatte::InputGetter::Button::A)
		.value("B", gambatte::InputGetter::Button::B)
		.value("SELECT", gambatte::InputGetter::Button::SELECT)
		.value("START", gambatte::InputGetter::Button::START)
		.value("RIGHT", gambatte::InputGetter::Button::RIGHT)
		.value("LEFT", gambatte::InputGetter::Button::LEFT)
		.value("UP", gambatte::InputGetter::Button::UP)
		.value("DOWN", gambatte::InputGetter::Button::DOWN)
	;

	enum_<gambatte::GB::cpu_register>("CPU_Register")
		.value("REG_CYCLECOUNTER", gambatte::GB::cpu_register::REG_CYCLECOUNTER)
		.value("REG_PC", gambatte::GB::cpu_register::REG_PC)
		.value("REG_SP", gambatte::GB::cpu_register::REG_SP)
		.value("REG_HF1", gambatte::GB::cpu_register::REG_HF1)
		.value("REG_HF2", gambatte::GB::cpu_register::REG_HF2)
		.value("REG_ZF", gambatte::GB::cpu_register::REG_ZF)
		.value("REG_CF", gambatte::GB::cpu_register::REG_CF)
		.value("REG_A", gambatte::GB::cpu_register::REG_A)
		.value("REG_B", gambatte::GB::cpu_register::REG_B)
		.value("REG_C", gambatte::GB::cpu_register::REG_C)
		.value("REG_D", gambatte::GB::cpu_register::REG_D)
		.value("REG_E", gambatte::GB::cpu_register::REG_E)
		.value("REG_F", gambatte::GB::cpu_register::REG_F)
	;

	class_<PyGambatteGB>("GB")
		.def("load", &PyGambatteGB::load)
		.def("reset", &PyGambatteGB::reset)
		.def("runFor", &PyGambatteGB::runFor)
		.def("run_frame", &PyGambatteGB::run_frame)
		.def("get_video", &PyGambatteGB::get_video)	
		.def("get_video2", &PyGambatteGB::get_video2)
		.def("set_input", &PyGambatteGB::set_input)
		.def("print_interrupt_timing", &PyGambatteGB::print_interrupt_timing)
		.def("get_cycle_counter", &PyGambatteGB::get_cycle_counter)
		.def("getWorkRam", &PyGambatteGB::getWorkRam)
		.def("getSaveRam", &PyGambatteGB::getSaveRam)
		.def("getVideoRam", &PyGambatteGB::getVideoRam)
		.def("getIoRam", &PyGambatteGB::getIoRam)
		.def("getRegister", &PyGambatteGB::getRegister)
		.def("setRegister", &PyGambatteGB::setRegister)
		.def("getSaveState", &PyGambatteGB::getSaveState)
		.def("loadSaveState", &PyGambatteGB::loadSaveState)
	;

}

#include "source.h"



void Source::set_antenna(std::string ant)
{
	antenna = ant;
	
}
std::string Source::get_antenna() {
	return antenna;
}
double Source::get_min_hz() {
	return min_hz;
}
double Source::get_max_hz() {
	return max_hz;
}
double Source::get_center() {
	return center;
}
double Source::get_rate() {
	return rate;
}
std::string Source::get_driver() {
	return driver;
}
std::string Source::get_device() {
	return device;
}
void Source::set_error(double e) {
	error = e;
}
double Source::get_error() {
	return error;
}
void Source::set_bb_gain(int b)
{
	if (driver == "osmosdr") {
		bb_gain = b;
		cast_to_osmo_sptr(source_block)->set_bb_gain(bb_gain);
	}
}
int Source::get_bb_gain() {
	return bb_gain;
}
void Source::set_gain(int r)
{
	if (driver == "osmosdr") {
		gain = r;
		cast_to_osmo_sptr(source_block)->set_gain(gain);
	}

}
int Source::get_gain() {
	return gain;
}
void Source::set_if_gain(int i)
{

}
int Source::get_if_gain() {
	return if_gain;
}
void Source::create_analog_recorders(gr::top_block_sptr tb, int r) {
	max_analog_recorders = r;

	for (int i = 0; i < max_analog_recorders; i++) {
		analog_recorder_sptr log = make_analog_recorder( center, center, rate, 0, i);
		analog_recorders.push_back(log);
		tb->connect(source_block, 0, log, 0);
	}
}
Recorder * Source::get_analog_recorder(int priority)
{
	for(std::vector<analog_recorder_sptr>::iterator it = analog_recorders.begin(); it != analog_recorders.end(); it++) {
		analog_recorder_sptr rx = *it;
		if (!rx->is_active())
		{
			return (Recorder *) rx.get();
			break;
		}
	}
	BOOST_LOG_TRIVIAL(info) << "[ " << driver << " ] No Analog Recorders Available";
	return NULL;

}
void Source::create_digital_recorders(gr::top_block_sptr tb, int r) {
	max_digital_recorders = r;

	for (int i = 0; i < max_digital_recorders; i++) {
#ifdef DSD
		dsd_recorder_sptr log = make_dsd_recorder( center, center, rate, 0, i);
#endif
        BOOST_LOG_TRIVIAL(info) << "created it!";
		digital_recorders.push_back(log);
        BOOST_LOG_TRIVIAL(info) << "pushed it!";
		tb->connect(source_block, 0, log, 0);
        BOOST_LOG_TRIVIAL(info) << "connected it!";
	}
}
void Source::create_debug_recorders(gr::top_block_sptr tb, int r) {
	max_debug_recorders = r;

	for (int i = 0; i < max_debug_recorders; i++) {

		debug_recorder_sptr log = make_debug_recorder( center, center, rate, 0, i);

		debug_recorders.push_back(log);
		tb->connect(source_block, 0, log, 0);
	}
}

Recorder * Source::get_debug_recorder()
{
	for(std::vector<debug_recorder_sptr>::iterator it = debug_recorders.begin(); it != debug_recorders.end(); it++) {
		debug_recorder_sptr rx = *it;
		if (!rx->is_active())
		{
			return (Recorder *) rx.get();
			break;
		}
	}
	//BOOST_LOG_TRIVIAL(info) << "[ " << driver << " ] No Debug Recorders Available";
	return NULL;

}

int Source::get_num_available_recorders() {
	int num_available_recorders = 0;
#ifdef DSD
	for(std::vector<dsd_recorder_sptr>::iterator it = digital_recorders.begin(); it != digital_recorders.end(); it++) {
		dsd_recorder_sptr rx = *it;
#endif
		if (!rx->is_active())
		{
			num_available_recorders++;
		}
	}
	return num_available_recorders;
}

Recorder * Source::get_digital_recorder(int priority)
{
	int num_available_recorders = get_num_available_recorders();
	//BOOST_LOG_TRIVIAL(info) << "\tTG Priority: "<< priority << " Available Digital Recorders: " <<num_available_recorders;

	if (priority> num_available_recorders) { // a low priority is bad. You need atleast the number of availalbe recorders to your priority
		//BOOST_LOG_TRIVIAL(info) << "Not recording because of priority";
		return NULL;
	}

#ifdef DSD
	for(std::vector<dsd_recorder_sptr>::iterator it = digital_recorders.begin(); it != digital_recorders.end(); it++) {
		dsd_recorder_sptr rx = *it;
#endif
		if (!rx->is_active())
		{
			return (Recorder *) rx.get();
			break;
		}
	}
	BOOST_LOG_TRIVIAL(info) << "[ " << driver << " ] No Digital Recorders Available";
	return NULL;

}
gr::basic_block_sptr Source::get_src_block() {
	return source_block;
}
Source::Source(double c, double r, double e, std::string drv, std::string dev)
{
	rate = r;
	center = c;
	error = e;
	min_hz = center - (rate/2);
	max_hz = center + (rate/2);
	driver = drv;
	device = dev;
	if (driver == "osmosdr") {
		osmosdr::source::sptr osmo_src;
		if (dev == "") {
			osmo_src = osmosdr::source::make();
		} else {
			osmo_src = osmosdr::source::make(dev);
		}
		BOOST_LOG_TRIVIAL(info) << "SOURCE TYPE OSMOSDR (osmosdr)";
		BOOST_LOG_TRIVIAL(info) << "Setting sample rate to: " << rate;
		osmo_src->set_sample_rate(rate);
		BOOST_LOG_TRIVIAL(info) << "Tunning to " << center + error << "hz";
		osmo_src->set_center_freq(center + error,0);
		source_block = osmo_src;
	}
}

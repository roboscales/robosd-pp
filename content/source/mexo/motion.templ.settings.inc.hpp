{
	{//cb
		{} //ref
	}
	, 0 //adjust_value
	, MOTION_PROP_GAIN //mexo_parametr_t propGain;
	, MOTION_MODEL_GAIN //mexo_parametr_t modelGain;
	, MOTION_CONTROL_SHIFT //uint8_t controlShift;
	, MOTION_MODEL_SHIFT//uint8_t modelShift;
	, MOTION_FORCE_GAIN //parameter_t forceGain;
	, MOTION_FORCE_LIM //signal_t forceLim;
}

#undef MOTION_PROP_GAIN
#undef MOTION_MODEL_GAIN
#undef MOTION_CONTROL_SHIFT
#undef MOTION_MODEL_SHIFT
#undef MOTION_FORCE_GAIN
#undef MOTION_FORCE_LIM

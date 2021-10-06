{
	{ //cb
		{} //ref
	}
	, {//qa
		{} //ref
		,0 // 
		,REGULATOR_PROP_GAIN //mexo_parametr_t propGain;
		,REGULATOR_MODEL_GAIN //mexo_parametr_t modelGain;
		,REGULATOR_DIFF_GAIN //mexo_parametr_t diffGain;
		,REGULATOR_CONTROL_SHIFT //uint8_t controlShift;
		,REGULATOR_MODEL_SHIFT//uint8_t modelShift;
	}
		, REGULATOR_RAMP_STEP
}
#undef REGULATOR_PROP_GAIN
#undef REGULATOR_MODEL_GAIN
#undef REGULATOR_DIFF_GAIN
#undef REGULATOR_CONTROL_SHIFT
#undef REGULATOR_MODEL_SHIFT
#undef REGULATOR_RAMP_STEP

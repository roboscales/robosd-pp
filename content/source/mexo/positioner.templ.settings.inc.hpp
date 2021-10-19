{
	{//cb
		{} //ref
	}
	, 0 //adjust_value
	, POSITIONER_PROP_GAIN //mexo_parametr_t propGain;
	, POSITIONER_DIFF_GAIN //mexo_parametr_t diffGain;
	, POSITIONER_CONTROL_SHIFT //uint8_t controlShift;
	, POSITIONER_DIFF_QUADR_GAIN //mexo_parametr_t diffQuadrGain;
	, POSITIONER_DIFF_QUADR_SHIFT//uint8_t diffQuadrShift;
	, POSITIONER_DEAD_ZONE //signal_t deadZone;
	, POSITIONER_CRAWL_SPEED //signal_t crawlSpeed;
}

#undef POSITIONER_PROP_GAIN
#undef POSITIONER_DIFF_GAIN
#undef POSITIONER_CONTROL_SHIFT
#undef POSITIONER_DIFF_QUADR_GAIN
#undef POSITIONER_DIFF_QUADR_SHIFT
#undef POSITIONER_DEAD_ZONE
#undef POSITIONER_CRAWL_SPEED

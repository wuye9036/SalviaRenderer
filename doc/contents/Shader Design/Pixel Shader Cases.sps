float return_if( bool v ){
	float f = 0.0;
	if (v){
		f = 2.5f;
		return f;
	}
	f += 2.2f;
	return f;
}

V.set( BET, VALUE );
V.get( BET, VALUE );

float return_if( bool v ){
	float __ret;
	float f;
	f.set_value(0.0f);
	
	f.set_mask_value(v, 2.5f);
	__ret.set_mask_value(v, f);
	
	float tmp = f.add( f.get_value(!v), 2.0f );
	f.set_value( !v, tmp );
	__ret.set_value(!v, f);
	
	return __ret.get_value(true);
}
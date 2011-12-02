struct In{
	float tex: TEXCOORD(0);
};

float ps_main( In in ): COLOR
{
	return in.tex;
}
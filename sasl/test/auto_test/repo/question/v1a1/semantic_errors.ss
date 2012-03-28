float test_no_overloaded_function()
{
	return cross(5);
}

struct not_a_member
{
	int f;
};

int test_not_a_member()
{
	not_a_member n;
	return n.g;
}

int3 test_invalid_swizzle()
{
	int2 x = int2(0, 5);
	return x.xyw;
}
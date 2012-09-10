/*
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

int test_member_left_must_have_struct()
{
	sampler s;
	return s.k;
}

int cond_expr_cannot_convert_from()
{
	sampler s;
	return s ? 1 : s;
}

int test_operator_unmathced_type()
{
	sampler s;
	float r;
	return 1 + s;
}

int test_undeclare_identifier()
{
	int f = not_a_member + 3;
	k t = 3;
	return x + 5;
}

struct redef;
struct redef
{};

struct redef;
 
struct redef
{ int d; };

int test_case_expr_type()
{
	int x = 5;
	switch( x )
	{
	case x:
	case 3.0f:
		break;
	}
	return 0;
}

int test_function_parameter_error( xx )
{
	return 0;
}
*/
void test_lvalue_error()
{
	int a = 3;
	int b = 5;
	int c = 0;

	++++a;
	a++;
	++(a+b);

	(a+b) += 5;
	7 += 3;
	++(a == 0 ? b : c);
	(a == 1 ? b : c++)++;
}
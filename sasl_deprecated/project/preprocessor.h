#ifndef SASL_PREPROCESSOR_H
#define SASL_PREPROCESSOR_H

/* 文件预处理 */
class preprocessor
{
	/**
	使用被处理的源文件名作为输入，并输出一个预处理后的临时文件。
	预处理后的文件除#line指令外不再携带其他预编译指令。
	该函数必须为线程安全的函数，可以多线程执行。
	**/
	virtual std::string preprocess(const std::string& filename) = 0;
};

#endif // preprocessor_H__
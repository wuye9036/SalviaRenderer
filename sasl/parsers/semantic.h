#ifndef SASL_SEMANTIC_H
#define SASL_SEMANTIC_H

struct semantic_symbols : symbols<>{
	semantic_symbols(){
	}
}

template<class ScannerT>
semantic::definition<ScannerT>::definition(const semantic& self){
	//r_start = 

}

template<class ScannerT>
semantic::definition<ScannerT>::start(){
	return r_start;
}

#endif
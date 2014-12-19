
#define MAX_LENGTH_OF_WORD 50
struct AWord {
	char word[ MAX_LENGTH_OF_WORD ];
	int length;
};
channel struct AWord WORD_CHANNEL   __attribute__((depth(50000))); 
channel struct AWord STRING_CMP_CHANNEL 	__attribute__((depth(4096)));
channel uint  STOP_L  __attribute__((depth(4))); 
channel uint  STOP_M  __attribute__((depth(4))); 

void stop_all() {
	write_channel_altera(STOP_L, 1);
	write_channel_altera(STOP_M, 1);
}


kernel void word_processor( __global const char* restrict text, const int length)
{
	bool stop_processing = false;
	struct AWord word;
	word.length = 0;

	#pragma unrolll MAX_LENGTH_OF_WORD 
	for( int idx = 0 ; idx<length; idx ++) {
		char ch = text[idx];
		if( ( ch == ' ' && word.length!= 0 ) || word.length > MAX_LENGTH_OF_WORD - 2 ) {
			word.word[ word.length++ ] = 0;
	//		printf(" send a word= %s\n", word.word );
			write_channel_altera( WORD_CHANNEL, word );
			word.length = 0;
		} else if( ch == ' ' /*&& word.length== 0 */) { 
		} else {
			word.word[ word.length++ ] = ch;
		}
	}
	stop_all();
}
kernel void compare_length_processing( const int length /*length of pattern */ )
{
	bool stop_processing = false;
	bool valid = false;
	while( true ) {
		__private struct AWord word =  read_channel_nb_altera(WORD_CHANNEL, &valid);
		if( valid && length == word.length ) {
			write_channel_altera( STRING_CMP_CHANNEL, word );	
		} else if( !valid && stop_processing ) {	
			break;
		}
		
		read_channel_nb_altera(STOP_L,&valid);
		if( valid )
			stop_processing = true;
	}
}

kernel void matching( __global const char* restrict pattern, const int length /*length of pattern */ , __global char* restrict result )
{
	bool stop_processing = false;
	int count = 0;
	bool valid = false;
	char _pattern[ MAX_LENGTH_OF_WORD ] ;
	for( int idx = 0; idx < length ; idx ++ ) {
		_pattern[idx] = pattern[idx];
	}
	while( true ) {
		__private struct AWord word =  read_channel_nb_altera( STRING_CMP_CHANNEL, &valid);
		if( !valid && stop_processing ) {	
			break;
		} else if ( valid ) {
			bool same = true;
			#pragma unroll 50
			for( int idx = 0 ; idx < 50; idx ++ )
				if( idx < length && _pattern[ idx ] != word.word[ idx ] ) 
					same = false;
			if( same )
					count ++;

		} 
		
		read_channel_nb_altera(STOP_M,&valid);
		if( valid )
			stop_processing = true;
	}
	result[ 0 ] = count;
}


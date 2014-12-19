
#define MAX_LENGTH_OF_WORD 50
struct AWord {
	char word[ MAX_LENGTH_OF_WORD ];
	int length;
};
channel struct AWord WORD_CHANNEL   __attribute__((depth(4096))); 

channel uint  STOP  __attribute__((depth(4))); 

void stop_all() {
	write_channel_altera(STOP, 1);
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
		__private struct AWord word =  read_channel_nb_altera(WORD_CHANNEL, &valid);
		if( !valid && stop_processing ) {	
			break;
		} else if ( !valid ) {
			// do nothing
		} else if( length == word.length ) {
			//printf(" is matching %s, %s\n", word.word, pattern );
			bool same = true;
			#pragma unrolll MAX_LENGTH_OF_WORD
			for( int idx = 0 ; idx < MAX_LENGTH_OF_WORD; idx ++ ) {
				if( idx < length && _pattern[idx] != word.word[idx] ) {
					same = false;
				}
			}
			if( same )
				count ++;
		} 
		
		read_channel_nb_altera(STOP,&valid);
		if( valid )
			stop_processing = true;
	}
	result[ 0 ] = count;
}



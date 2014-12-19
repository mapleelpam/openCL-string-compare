
#define MAX_LENGTH_OF_WORD 50
struct AWord {
	char word[ MAX_LENGTH_OF_WORD ];
	int length;
};
channel struct AWord WORD_CHANNEL   __attribute__((depth(4096))); 

channel uint  STOP  __attribute__((depth(4))); 

kernel void stop_all() {
//  write_channel_altera(STOP1, 1);
//  write_channel_altera(STOP2, 1);
}


kernel void word_processor( __global const char *text, const int length)
{
	bool stop_processing = false;
	struct AWord word;
	word.length = 0;
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
	write_channel_altera(STOP, 1);
}

kernel void matching( __global const char *pattern, const int length /*length of pattern */ , __global char *result )
{
	bool stop_processing = false;
	while( true ) {
		bool valid = false;
		__private struct AWord word =  read_channel_nb_altera(WORD_CHANNEL, &valid);
		if( valid ) {	
		//	printf(" recv word = %s\n", word.word);
			if( length == word.length ) {
		//		printf(" is matching %s, %s\n", word.word, pattern );
				bool same = true;
				for( int idx = 0 ; idx < length ; idx ++ ) {
					if( word.word[idx] != pattern[idx] ) {
						same = false;
					}
				}
				if( same )
					result[ 0 ] ++;	
			} 
		} else {

			if( stop_processing == false )
				read_channel_nb_altera(STOP,&stop_processing);
			if(stop_processing ) {
				break;
			}
		}
	}
}

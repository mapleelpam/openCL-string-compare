
#define MAX_LENGTH_OF_WORD 50
struct AWord {
	char word[ MAX_LENGTH_OF_WORD ];
	int length;
};
channel char TEXT_CHANNEL   __attribute__((depth(10240))); 
channel struct AWord WORD_CHANNEL   __attribute__((depth(1024))); 

channel uint  STOP1  __attribute__((depth(4))); 
channel uint  STOP2  __attribute__((depth(4))); 

kernel void stop_all() {
//  write_channel_altera(STOP1, 1);
//  write_channel_altera(STOP2, 1);
}

kernel void text_processor( __global const char *text, const int length)
{
	int idx = 0;
	struct AWord word;
	for( idx = 0 ; idx<length; idx ++) {
		write_channel_altera( TEXT_CHANNEL, text[idx] );
//		printf(" text[%d] = %c\n", idx, text[idx] );
	}
	// stop_all();
	write_channel_altera(STOP1, 1);
	write_channel_altera(STOP2, 1);
}

kernel void word_processor( )
{
	bool stop_processing = false;
	struct AWord word;
	int idx = 0;
	while( 1 ) {
		bool valid = false;
		char ch = read_channel_nb_altera( TEXT_CHANNEL, &valid );
		if( valid ){
			if( ch == ' ' && idx != 0 ) {
				word.length = idx+1;
				word.word[ idx ] = 0;
				printf(" send a word= %s\n", word.word );
				write_channel_altera( WORD_CHANNEL, word );
				idx = 0;
			} else {
				word.word[ idx ] = ch;
				idx ++;
			}
		} else {

			if( stop_processing == false )
				read_channel_nb_altera(STOP1, &stop_processing);
			if( stop_processing ){
				break;
			}
		}
	}
}

kernel void matching( __global const char *pattern, const int length /*length of pattern */ ) //, __global const char *result )
{
	bool match = false;
	bool stop_processing = false;
	while( true ) {
		bool valid = false;
		__private struct AWord word =  read_channel_nb_altera(WORD_CHANNEL, &valid);
		if( valid ) {	
			printf(" recv word = %s\n", word.word);
			if( length == word.length ) {
				printf(" is matching %s, %s\n", word.word, pattern );
				bool same = true;
				for( int idx = 0 ; idx < length ; idx ++ ) {
					if( word.word[idx] != pattern[idx] ) {
						same = false;
					}
				}
				match = true;
				
			} 
		} else {

			if( stop_processing == false )
				read_channel_nb_altera(STOP2,&stop_processing);
			if(stop_processing ) {
				break;
			}
		}
	}
}

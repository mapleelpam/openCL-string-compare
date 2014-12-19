
#define MAX_LENGTH_OF_WORD 50
#define NUM_OF_TASK 4

struct AWord {
	char word[ MAX_LENGTH_OF_WORD ];
	int length;
};
channel struct AWord WORD_CHANNEL[NUM_OF_TASK]   __attribute__((depth(4096))); 

channel uint  STOP[NUM_OF_TASK]  __attribute__((depth(4))); 

kernel void stop_all() {
	for( int idx = 0 ; idx < NUM_OF_TASK ; idx++ ) 
		write_channel_altera( STOP[idx], 1 );
}


kernel void word_processor( __global const char *restrict text, const int length)
{
	bool stop_processing = false;
	struct AWord word;
	word.length = 0;
	for( int idx = 0 ; idx<length; idx ++) {
		char ch = text[idx];
		if( ( ch == ' ' && word.length!= 0 ) || word.length > MAX_LENGTH_OF_WORD - 2 ) {
			word.word[ word.length++ ] = 0;
//			printf(" send a word= %s\n", word.word );
			write_channel_altera( WORD_CHANNEL[ idx % NUM_OF_TASK ], word );
			word.length = 0;
		} else if( ch == ' ' /*&& word.length== 0 */) { 
		} else {
			word.word[ word.length++ ] = ch;
		}
	}
	stop_all();
}

__attribute__((num_compute_units(NUM_OF_TASK)))
__attribute__((num_simd_work_items(NUM_OF_TASK)))
__attribute__( (reqd_work_group_size( NUM_OF_TASK, 1, 1 ) ) )
kernel void matching( __global const char *restrict pattern, const int length /*length of pattern */ , __global char* restrict result )
{
	bool stop_processing = false;
	int pn = get_global_id(0);
	// printf(" matching %d \n", pn );
	while( true ) {
		bool valid = false;
		__private struct AWord word =  read_channel_nb_altera(WORD_CHANNEL[pn], &valid);
		if( valid ) {	
		//	printf(" %d recv word = %s\n", pn,word.word);
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
				read_channel_nb_altera(STOP[pn],&stop_processing);
			if(stop_processing ) {
				break;
			}
		}
	}
}

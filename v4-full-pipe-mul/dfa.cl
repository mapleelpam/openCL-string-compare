
#define MAX_LENGTH_OF_WORD 50
#define NUM_OF_TASK 4

struct AWord {
	char word[ MAX_LENGTH_OF_WORD ];
	int length;
};
channel struct AWord WORD_CHANNEL[NUM_OF_TASK]    __attribute__((depth(50000))); 
channel struct AWord STRING_CMP_CHANNEL[NUM_OF_TASK]  	__attribute__((depth(4096)));
channel uint  STOP_L[NUM_OF_TASK]   __attribute__((depth(4))); 
channel uint  STOP_M[NUM_OF_TASK]   __attribute__((depth(4))); 

void stop_all() {
	for( int idx = 0 ; idx < NUM_OF_TASK ; idx ++ ) {
		write_channel_altera(STOP_L[idx], 1);
		write_channel_altera(STOP_M[idx], 1);
	}
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
			write_channel_altera( WORD_CHANNEL[idx%NUM_OF_TASK], word );
			word.length = 0;
		} else if( ch == ' ' /*&& word.length== 0 */) { 
		} else {
			word.word[ word.length++ ] = ch;
		}
	}
	stop_all();
}

__attribute__((num_compute_units(NUM_OF_TASK)))
//__attribute__((num_simd_work_items(NUM_OF_TASK)))
__attribute__( (reqd_work_group_size( NUM_OF_TASK, 1, 1 ) ) )
kernel void compare_length_processing( const int length /*length of pattern */ )
{
	bool stop_processing = false;
	bool valid = false;
	int pn = get_global_id(0);
	while( true ) {
		__private struct AWord word =  read_channel_nb_altera(WORD_CHANNEL[pn], &valid);
		if( valid && length == word.length ) {
			write_channel_altera( STRING_CMP_CHANNEL[pn], word );	
		} else if( !valid && stop_processing ) {	
			break;
		}
		
		read_channel_nb_altera(STOP_L[pn],&valid);
		if( valid )
			stop_processing = true;
	}
}

__attribute__((num_compute_units(NUM_OF_TASK)))
//__attribute__((num_simd_work_items(NUM_OF_TASK)))
__attribute__( (reqd_work_group_size( NUM_OF_TASK, 1, 1 ) ) )
kernel void matching( __global const char* restrict pattern, const int length /*length of pattern */ , __global char* restrict result )
{
	bool stop_processing = false;
	int count = 0;
	bool valid = false;
	char _pattern[ MAX_LENGTH_OF_WORD ] ;
	for( int idx = 0; idx < length ; idx ++ ) {
		_pattern[idx] = pattern[idx];
	}
	int pn = get_global_id(0);
	while( true ) {
		__private struct AWord word =  read_channel_nb_altera( STRING_CMP_CHANNEL[pn], &valid);
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
		
		read_channel_nb_altera(STOP_M[pn],&valid);
		if( valid )
			stop_processing = true;
	}
	result[ 0 ] += count;
}


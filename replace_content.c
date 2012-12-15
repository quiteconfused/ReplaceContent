#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <curl/curl.h> //your directory may be different
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

struct entry {
   char* string;
   unsigned int item_size;
   TAILQ_ENTRY(entry) entries;         /* Tail queue. */
};

size_t writeCallback(char* buf, size_t size, size_t nmemb, void *head)
{ //callback must have this declaration
    //buf is a pointer to the data that curl has for us
    //size*nmemb is the size of the buffer
    
    struct entry *n1;
    
    n1 = (struct entry*)malloc(sizeof(struct entry));
    n1->string = (char*)malloc(size*nmemb+1);
    n1->item_size=size*nmemb;
    memset(n1->string, 0, size*nmemb+1);
    memcpy(n1->string, buf, size*nmemb);
   	TAILQ_INSERT_TAIL((TAILQ_HEAD(, entry)*)head, n1, entries);
 	
    return size*nmemb; //tell curl how many bytes we handled
}
 
int setup_request3(unsigned char* request, void* head)
{
	CURL *curl;
	CURLcode res;
	unsigned char partial_request[] = "http://images.google.com/images?q=%s&gbv=2&start=0&hl=en&ie=UTF-8&safe=off&sa=N";
	unsigned char* actual_request = malloc(strlen(partial_request)+strlen(request)+1);

	memset(actual_request, 0, strlen(partial_request)+strlen(request)+1);

	sprintf(actual_request, partial_request, request);
 
	/* In windows, this will init the winsock stuff */ 
	//curl_global_init(CURL_GLOBAL_ALL);
 
	/* get a curl handle */ 
	curl = curl_easy_init();
	if(curl) {
		/* First set the URL that is about to receive our POST. This URL can
		just as well be a https:// URL if that is what should receive the
		data. */ 
		curl_easy_setopt(curl, CURLOPT_URL, actual_request);
		curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt (curl, CURLOPT_WRITEDATA, head);
//		curl_easy_setopt (curl, CURLOPT_HEADER, 0);
//		curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
//		curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 20 );
		curl_easy_setopt (curl, CURLOPT_USERAGENT, "Mozilla");
//		curl_easy_setopt (curl, CURLOPT_REFERER, "http://images.google.com/");
		/* Now specify the POST data */ 
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, actual_request);
 
		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
 	
		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	free(actual_request);
	return 0;
}

unsigned int send_results_to_server(char* hostname, char* port, char* results){
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    portno = atoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        fprintf(stderr, "Error opening socket\n");
        return 1;
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        return 1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    	fprintf(stderr, "error connecting\n");
    	return 1;
    }

    n = write(sockfd,results,strlen(results));
    if (n < 0){
         fprintf(stderr, "Error writing to socket\n");
         return 1;
    }
    
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) {
         fprintf(stderr, "Error reading from socket\n");
         return 1;
    }
    fprintf(stderr, "%s\n",buffer);
    close(sockfd);
    return 0;
}

void setup_request2(CURL* curl, char *request, void *head){
    struct curl_httppost* post=NULL;
    struct curl_httppost* last=NULL;

	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, head);
	
	curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com/searchbyimage");
	curl_easy_setopt (curl, CURLOPT_HEADER, 0);
	curl_easy_setopt (curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; rv:8.0) Gecko/20100101 Firefox/8.0");
	curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 20 );
	curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt (curl, CURLOPT_REFERER, "http://www.google.com/");
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "btnl", CURLFORM_COPYCONTENTS, "Search", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "hl", CURLFORM_COPYCONTENTS, "en", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "safe", CURLFORM_COPYCONTENTS, "off", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "q", CURLFORM_COPYCONTENTS, request, CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "tbm", CURLFORM_COPYCONTENTS, "isch", CURLFORM_END);

	curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
}



void setup_request(CURL* curl, char *filename, void *head){
    struct curl_httppost* post=NULL;
    struct curl_httppost* last=NULL;

	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, head);
	
	curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com/searchbyimage/upload");
	curl_easy_setopt (curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; rv:8.0) Gecko/20100101 Firefox/8.0");
	curl_easy_setopt (curl, CURLOPT_HEADER, 0);
	curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 20 );
	curl_easy_setopt (curl, CURLOPT_REFERER, "http://images.google.com/");

	curl_formadd(&post, &last, CURLFORM_COPYNAME, "image_url", CURLFORM_COPYCONTENTS, "" , CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "btnG", CURLFORM_COPYCONTENTS, "Search", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "encoded_image", CURLFORM_FILE, filename , CURLFORM_CONTENTTYPE, "image/jpeg", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "image_content", CURLFORM_COPYCONTENTS, "", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "filename", CURLFORM_COPYCONTENTS, "",CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "hl", CURLFORM_COPYCONTENTS, "en", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "safe", CURLFORM_COPYCONTENTS, "off", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "bih", CURLFORM_COPYCONTENTS, "", CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "biw", CURLFORM_COPYCONTENTS, "", CURLFORM_END);	

	curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
}

#define DELIM "<>"

void get_info(char* filename, unsigned int option, void *head){
    CURL* curl; //our curl object

	curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
	curl = curl_easy_init();
	setup_request(curl, filename, head);
	curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

void get_info2(char* request, unsigned int option, void *head){
    CURL* curl; //our curl object

	curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
	curl = curl_easy_init();
	setup_request2(curl, request, head);
	curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

void entry_point(char* filename, unsigned char* servername, unsigned char* port, unsigned char* completed_string, unsigned int size){
    TAILQ_HEAD(, entry) head;
	struct entry *np;
	int best_guess_offset=-1;
	unsigned int recieved_size = 0;
	unsigned int current_loc = 0; 
	char* complete_html = 0, *buf=0, *complete_html_copy=0, *visually_similar_offset=0, *previous_buffer_offset=0;   
	unsigned int scanlinks=0;


	TAILQ_INIT(&head);                      /* Initialize the queue. */

    get_info(filename,0, &head);	
	
	for (np = head.tqh_first; np != NULL; np = np->entries.tqe_next)
		recieved_size+=np->item_size;
		
	complete_html = (char*)malloc(recieved_size+1);
	complete_html_copy = (char*)malloc(recieved_size+1);
	memset(complete_html, 0, recieved_size+1);
	memset(complete_html_copy, 0, recieved_size+1);
	
	for (np = head.tqh_first; np != NULL; np = np->entries.tqe_next){
		if(recieved_size && np->item_size){
			memcpy(complete_html+current_loc, np->string, np->item_size);
			memcpy(complete_html_copy+current_loc, np->string, np->item_size);
			current_loc+=np->item_size;
		}
	}
	if(1){
		char* tempptr=NULL;
		int notcomplete=1;
		previous_buffer_offset = complete_html;	
		for(buf = strtok_r(complete_html, DELIM, &tempptr); buf!=NULL && notcomplete; buf = strtok_r(NULL, DELIM, &tempptr)){
			if(!memcmp(buf, "Best guess for this image:", sizeof("Best guess for this image:")-1)){
				best_guess_offset=0;
			} else if(best_guess_offset==0){
				best_guess_offset++;
			} else if(best_guess_offset==1){
				sprintf(completed_string, "%s", buf);
				notcomplete=0;
			} else if(!memcmp(buf, "Visually similar", sizeof("Visually similar")-1)){
				visually_similar_offset = complete_html_copy + (buf - complete_html_copy);
				while(memcmp(visually_similar_offset, "q=", 2) && visually_similar_offset>previous_buffer_offset)
					visually_similar_offset--;
				if(previous_buffer_offset!=visually_similar_offset){
					unsigned char *copy_completed_string=completed_string;
					visually_similar_offset+=2;
					while(	memcmp(visually_similar_offset, "&amp", 4) && 
						copy_completed_string - completed_string < size && 
						visually_similar_offset < buf+strlen(buf)){
						if(visually_similar_offset[0]=='+'){
							sprintf(copy_completed_string, "%c", ' ');
							copy_completed_string++;
						} else if(!memcmp(visually_similar_offset, "&#39;", 5)){
							visually_similar_offset+=4;
							sprintf(copy_completed_string, "%c", '\'');
							copy_completed_string++;
						}
						else{
							sprintf(copy_completed_string, "%c", visually_similar_offset[0]);
							copy_completed_string++;
						}
						visually_similar_offset++;
					}
					fprintf(stderr, "%s: %s\n", filename, completed_string);
					if(servername!=NULL && port!=NULL)
						send_results_to_server(servername, port, completed_string);
					notcomplete=0;
				}
				else {
					notcomplete=0;
				}
			}
			previous_buffer_offset = buf;
		}
	}
		

	while (head.tqh_first != NULL){
		np = head.tqh_first;
		free(np->string);
		TAILQ_REMOVE(&head, head.tqh_first, entries);
		free(np);
	}
	
	
	free(complete_html_copy);
	free(complete_html);
}

#define DELIM3 "\%&= <\">"

void entry_point2(char* request, unsigned char* servername, unsigned char* port, unsigned char* completed_string, unsigned int size){
    TAILQ_HEAD(, entry) head;
	struct entry *np;
	int best_guess_offset=-1;
	unsigned int recieved_size = 0;
	unsigned int current_loc = 0; 
	char* complete_html = 0, *buf=0, *complete_html_copy=0, *visually_similar_offset=0, *previous_buffer_offset=0;   
	unsigned int scanlinks=0;


	TAILQ_INIT(&head);                      /* Initialize the queue. */

    setup_request3(request, &head);	
	//get_info2(request, 0, &head);
	
	for (np = head.tqh_first; np != NULL; np = np->entries.tqe_next)
		recieved_size+=np->item_size;
		
	complete_html = (char*)malloc(recieved_size+1);
	complete_html_copy = (char*)malloc(recieved_size+1);
	memset(complete_html, 0, recieved_size+1);
	memset(complete_html_copy, 0, recieved_size+1);
	
	for (np = head.tqh_first; np != NULL; np = np->entries.tqe_next){
		if(recieved_size && np->item_size){
			memcpy(complete_html+current_loc, np->string, np->item_size);
			memcpy(complete_html_copy+current_loc, np->string, np->item_size);
			current_loc+=np->item_size;
		}
	}
	
	if(0){
    	FILE* file = fopen("tmpfile.html", "w+");
    	fwrite(complete_html, recieved_size, 1, file);
    	fclose(file);
	}

	if(1){
		char* tempptr=NULL;
		previous_buffer_offset = complete_html;	
		for(buf = strtok_r(complete_html, DELIM3, &tempptr); buf!=NULL; buf = strtok_r(NULL, DELIM3, &tempptr)){
			if((strcasestr(buf, "http://")!=NULL || strcasestr(buf, "https://")!=NULL) && (strcasestr(buf, ".jpg")!=NULL || strcasestr(buf, ".png")!=NULL || strcasestr(buf, ".gif")!=NULL || strcasestr(buf, ".bmp")!=NULL)){
				printf("%s\n", buf);
				//break;
			}
		}
	}
		

	while (head.tqh_first != NULL){
		np = head.tqh_first;
		free(np->string);
		TAILQ_REMOVE(&head, head.tqh_first, entries);
		free(np);
	}
	
	
	free(complete_html_copy);
	free(complete_html);
}



void print_usage(int argc, char** argv){
	unsigned int x=0;
	fprintf(stderr, "%s: <Incoming Image> <Criteria File>\n", argv[0]);
	fprintf(stderr, "User supplied the following arguements\n");
	for(x=0;x<argc;x++)
		fprintf(stderr, "arg[%d]: %s\n", x, argv[x]);
}


#define DELIM2 " "

unsigned int is_picture_associated_with_content(unsigned char* result, unsigned char* incoming_string_file, unsigned char** outgoing_string){
	
	unsigned int answer = 0;
	
	FILE *compare_file = NULL;
	unsigned char* compare_buffer = NULL;
	unsigned int compare_buffer_size = 0;
	unsigned char* super_tok = NULL;
	char* super_tok_t = NULL;

	compare_file = fopen(incoming_string_file, "r");
	if(compare_file==NULL)
		return 0;

	fseek(compare_file, 0, SEEK_END);
	compare_buffer_size = ftell(compare_file);

	rewind(compare_file);
	
	compare_buffer = malloc(compare_buffer_size+1);
	memset(compare_buffer, 0, compare_buffer_size+1);
	fread(compare_buffer, 1, compare_buffer_size, compare_file);

	fclose(compare_file);
	

	for(super_tok = strtok_r(compare_buffer, "\n", &super_tok_t); super_tok!=NULL; super_tok = strtok_r(NULL, "\n", &super_tok_t)){	
		unsigned char* incoming_string = malloc(strlen(super_tok)+1);
		unsigned char* buffer = NULL;
		float percentage_match; 
		float fresult = 0;


		*outgoing_string = malloc(strlen(super_tok)+1);

		memset(incoming_string, 0, strlen(super_tok)+1);
		memset(*outgoing_string, 0, strlen(super_tok)+1);
		sscanf(super_tok, "%f %s %s", &percentage_match, incoming_string, *outgoing_string);

		buffer = malloc(strlen(incoming_string)+1);
		memset(buffer, 0, strlen(incoming_string)+1);
		memcpy(buffer, incoming_string, strlen(incoming_string));
		
		if(strlen(result)>0){
			unsigned char* buffer2 = malloc(strlen(result)+1);
			unsigned char* tok2=NULL;
			char* tok2_t=NULL;
			unsigned int number_of_total_words=0;
			unsigned int number_of_matches=0;
		
			memset(buffer2, 0, strlen(result)+1);
			memcpy(buffer2, result, strlen(result));
		
			for(tok2 = strtok_r(buffer2, " ", &tok2_t); tok2 != NULL; tok2 = strtok_r(NULL, " ", &tok2_t)){
				unsigned char* tok=NULL;
		        char* tok_t=NULL;
				number_of_total_words++;
				memcpy(buffer, incoming_string, strlen(incoming_string));
				for(tok = strtok_r(buffer, ",", &tok_t); tok != NULL; tok = strtok_r(NULL, ",", &tok_t)){
				
	
					if(!strcmp(tok2, tok)){
						number_of_matches++;
					}
				}
			}
		
			fresult = (float)number_of_matches / (float)number_of_total_words;
			
			fprintf(stderr, "Result: %s, Comparison: %s, Percentage Match: %f\n", result, incoming_string, fresult);
			
			answer = ( fresult >= percentage_match) ? 1 : 0;
		
			free(buffer2);
		}
		free(incoming_string);
		free(buffer);

		if(answer)
			break;

		//this is done so that we can use the string later and not reread the file ... uselessly
		free(*outgoing_string);
		*outgoing_string=NULL;
	}

	free(compare_buffer);

	return answer;
}
unsigned char* get_picture_name_associated_with_destination(unsigned char* outgoing_string){
	unsigned int size = 2000;
	unsigned char* buffer = malloc(size);
	memset(buffer, 0, size);

	entry_point2(outgoing_string, NULL, NULL, buffer, size);
}

int main(int argc, char** argv){

	unsigned int result_size = 2000;
	unsigned char* result = malloc(result_size);
	unsigned char* outgoing_string = NULL;
	unsigned int line_matching = 0;
	if(argc!=3){	
		print_usage(argc, argv);
		exit(1);
	}

	memset(result, 0, result_size);
	entry_point(argv[1], NULL, NULL, result, result_size);

	line_matching = is_picture_associated_with_content(result, argv[2], &outgoing_string);
	if(line_matching){
		fprintf(stderr, "Found match on line %d\n", line_matching);
		get_picture_name_associated_with_destination(outgoing_string);
	}

	free(result);
	if(outgoing_string!=NULL)
		free(outgoing_string);
	return 0;
}

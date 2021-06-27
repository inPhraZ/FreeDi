#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <curl/curl.h>

#include "cJSON.h"

#define API_URL		"https://api.dictionaryapi.dev/api/v2/entries/"

#define NUM_LANG	5
#define MAX_LANG	7
#define MAX_WORD	50
#define URL_LEN		100

static const char *languages[] = {
	"en_US",
	"en_GB",
	"de",
	"fr",
	"it"
};

struct data{
	char *response;
	size_t size;
};

void print_usage(int exit_code)
{
	printf("\nUsage:  freedi   [OPTION]   [WORD]\n");
	printf("List information about the WORD. (part of speech, definition, example, synonyms)\n");
	printf("If no language is specified, en_US is the default\n\n");
	printf("Options:\n");
	printf("-h, --help        display this help and exit\n");
	printf("-l, --language    language selection\n\n");
	printf("Supported languages:\n");
	printf("en_US              English(US) (default)\n");
	printf("en_GB              English(UK)\n");
	printf("de                 German\n");
	printf("fr                 French\n");
	printf("it                 Italian\n");

	printf("\nExample:  freedi  -l de hallo\n\n");

	exit(exit_code);
}

void curl_error_handler(CURL *handler, int res)
{
	fprintf(stderr, "Error: %s\n", curl_easy_strerror(res));
	curl_easy_cleanup(handler);
	exit(2);
}

void json_error_handler()
{
	const char *error_ptr;
	error_ptr = cJSON_GetErrorPtr();
	if (error_ptr)
		fprintf(stderr, "Error: %s\n", error_ptr);
	else
		fprintf(stderr, "Error: Something went wrong\n");
	exit(1);
}

size_t write_callback(void *data, size_t size, size_t nmemb, void *userdata)
{
	size_t numread = size * nmemb;
	struct data *d = (struct data *)userdata;

	char *ptr = (char *)realloc(d->response, d->size + numread + 1);
	if (!ptr)
		return 0;
	
	d->response = ptr;
	memcpy(&(d->response[d->size]), data, numread);
	d->size += numread;
	d->response[d->size] = 0;

	return numread;
}

void get_synonyms(cJSON *meaning)
{
	cJSON *definitions = cJSON_GetObjectItemCaseSensitive(meaning, "definitions");
	cJSON *definition = cJSON_GetArrayItem(definitions, 0);
	cJSON *synonyms = cJSON_GetObjectItemCaseSensitive(definition, "synonyms");
	cJSON *syn;
	int i = 0;
	printf("Synonyms: ");
	cJSON_ArrayForEach(syn, synonyms){
		if (cJSON_IsString(syn) && (syn->valuestring != NULL)){
			printf("%s", syn->valuestring);
			if (i != cJSON_GetArraySize(synonyms) - 1)	printf(", ");
		}
		i++;
	}
	printf("\n\n");
}

void get_definitions(cJSON *meaning)
{
	cJSON *definitions = cJSON_GetObjectItemCaseSensitive(meaning, "definitions");
	cJSON *definition;
	cJSON_ArrayForEach(definition, definitions){
		cJSON *def = cJSON_GetObjectItemCaseSensitive(definition, "definition");
		if (cJSON_IsString(def) && (def->valuestring != NULL)){
			printf("--> %s\n", def->valuestring);
		}
		cJSON *example = cJSON_GetObjectItemCaseSensitive(definition, "example");
		if (cJSON_IsString(example) && (example->valuestring != NULL)){
			printf("Example: %s\n", example->valuestring);
		}
	}

	printf("\n");
}

void get_meanings(cJSON *object)
{
	cJSON *meanings = cJSON_GetObjectItemCaseSensitive(object, "meanings");
	cJSON *meaning = cJSON_GetArrayItem(meanings, 0);
	cJSON *copy = meaning;
	printf("\n");
	cJSON_ArrayForEach(meaning, meanings){
		cJSON *pos = cJSON_GetObjectItemCaseSensitive(meaning, "partOfSpeech");
			if (cJSON_IsString(pos) && (pos->valuestring != NULL)){
				printf("(%s)\n", pos->valuestring);
			}
			get_definitions(meaning);
	}
	get_synonyms(copy);
}

void parse_json(struct data *json_data)
{
	cJSON *array;

	array = cJSON_Parse(json_data->response);
	if (!array)
		json_error_handler();
	if (cJSON_IsArray(array)){
		cJSON *object = cJSON_GetArrayItem(array, 0);
		cJSON *word = cJSON_GetObjectItemCaseSensitive(object, "word");
		if (cJSON_IsString(word) && (word->valuestring != NULL))
			printf("Word: %s\n", word->valuestring);
		get_meanings(object);
	}
	else{
		cJSON *title = cJSON_GetObjectItemCaseSensitive(array, "title");
		if (cJSON_IsString(title) && (title->valuestring != NULL))
			printf("%s\n", title->valuestring);
		else
			fprintf(stderr, "Invalid response!\n");
	}

	cJSON_Delete(array);
}

int validate_language(const char *lang)
{
	int i;
	for (i = 0; i < NUM_LANG; ++i){
		if (strncmp(lang, languages[i], MAX_LANG) == 0)
			return 1;
	}
	return 0;
}

void parse_arguments(int argc, char **argv, char *word, char *lang)
{
	int c;
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"language", required_argument, 0, 'l'},
		{0, 0, 0, 0}
	};

	while((c = getopt_long(argc, argv, "hl:", long_options, NULL)) != -1){
		switch(c){
			case 'h':
				print_usage(0);
				break;
			case 'l':
				if (validate_language(optarg))
					strncpy(lang, optarg, MAX_LANG);
				else
					fprintf(stderr, "Invalid language: %s\n", optarg);
				break;
			default:
				print_usage(1);
				break;
		}
	}
	if (argv[optind])
		strncpy(word, argv[optind], MAX_WORD);
	else{
		fprintf(stderr, "No words found\n\n");
		print_usage(1);
	}
}

void final_url(char *url, const char *word, char *lang)
{
	strncpy(url, API_URL, URL_LEN);
	strncat(lang, "/", 2);
	strncat(url, lang, MAX_LANG);
	strncat(url, word, MAX_WORD);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		print_usage(1);

	char lang[MAX_LANG];
	char word[MAX_WORD];
	char url[URL_LEN];

	memset(lang, 0, MAX_LANG);
	memset(word, 0, MAX_WORD);
	memset(url, 0, URL_LEN);

	parse_arguments(argc, argv, word, lang);

	if (strncmp(lang, "", 1) == 0)
		strncpy(lang, "en_US", MAX_LANG);

	final_url(url, word, lang);

	int res;
	struct data json = {
		.response = NULL,
		.size = 0
	};

	CURL *handler;
	handler = curl_easy_init();
	
	if (!handler){
		fprintf(stderr, "Error: Something went wrong\n");
		exit(2);
	}

	res = curl_easy_setopt(handler, CURLOPT_URL, url);
	if (res != CURLE_OK)
		curl_error_handler(handler, res);

	res = curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, write_callback);
	if (res != CURLE_OK)
		curl_error_handler(handler, res);

	res = curl_easy_setopt(handler, CURLOPT_WRITEDATA, (void *)&json);
	if (res != CURLE_OK)
		curl_error_handler(handler, res);

	res = curl_easy_perform(handler);
	if (res != CURLE_OK)
		curl_error_handler(handler, res);

	curl_easy_cleanup(handler);

	parse_json(&json);

	return 0;
}

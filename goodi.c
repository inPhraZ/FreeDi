#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define API_URL		"https://api.dictionaryapi.dev/api/v2/entries/en_US/"

void print_usage(int exit_code)
{
	printf("Usage:  goodi	[word]\n");
	exit(exit_code);
}

void curl_error_handler(CURL *handler, int res)
{
	fprintf(stderr, "Error: %s\n", curl_easy_strerror(res));
	curl_easy_cleanup(handler);
	exit(2);
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	return size * nmemb;
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		print_usage(1);

	char url[1024];
	int res;
	CURL *handler;
	handler = curl_easy_init();
	
	if (!handler){
		fprintf(stderr, "Error: Something went wrong\n");
		exit(2);
	}

	strcpy(url, API_URL);
	strcat(url, argv[1]);

	res = curl_easy_setopt(handler, CURLOPT_URL, url);
	if (res != CURLE_OK)
		curl_error_handler(handler, res);

	res = curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, write_callback);
	if (res != CURLE_OK)
		curl_error_handler(handler, res);

	res = curl_easy_perform(handler);
	if (res != CURLE_OK)
		curl_error_handler(handler, res);

	curl_easy_cleanup(handler);

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define API_URL		"https://api.dictionaryapi.dev/api/v2/entries/en_US/"

struct data{
	char *response;
	size_t size;
};

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

int main(int argc, char *argv[])
{
	if (argc == 1)
		print_usage(1);

	char url[1024];
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

	strcpy(url, API_URL);
	strcat(url, argv[1]);

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

	return 0;
}

enum webfile {
	JS,
	CSS,
	HTML,
	UNK
};

char *read_file(char *pathname, const char *static_files_loc, size_t *outlen);
enum webfile is_web_file(const char *filename);

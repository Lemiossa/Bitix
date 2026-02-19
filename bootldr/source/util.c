/************************************
 * util.c                           *
 * Criado por Matheus Leme Da Silva *
 ***********************************/

char to_upper(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - ('a' - 'A');
	return c;
}

char to_lower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	return c;
}

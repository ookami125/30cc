void printf(char *, ...);

int a() {
    printf("a\n");
    return 0;
}

int b() {
    printf("b\n");
    return 1;
}

int main()
{
    printf("a && b\n");
    if(a() && b())
    {
        printf("This shouldn't print!\n");
        return 1;
    }
    printf("b || a\n");
    if(b() || a())
    {
        printf("This should print !\n");
        return 0;
    }
    return 1;
}
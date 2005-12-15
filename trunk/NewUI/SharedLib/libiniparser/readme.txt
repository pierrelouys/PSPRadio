http://ndevilla.free.fr/iniparser/html/index.html
    |
    +-- Pizza
    |     +-- Cheese
    |     +-- Ham
    +-- Wine
         +--- Year
         +--- Grape
    

In an ini file, that would be converted to:

    [Pizza]

    [Pizza/Cheese]
    Name   = Gorgonzola ;
    Origin = Italy ;

    [Pizza/Ham]
    Name   = Parma ;
    Origin = Italy ;

    [Wine]

    [Wine/Year]
    Value = 1998 ;

    [Wine/Grape]
    Name   = Cabernet Sauvignon ;
    Origin = Chile ;
    
Accessing the above tree would give something like (error checking removed for clarity sake):

    dictionary * d ;

    d = iniparser_load("example.ini");

    printf("cheese name is %s\n", iniparser_getstr(d, "pizza/cheese:name"));
    printf("grape name is %s\n",  iniparser_getstr(d, "wine/grape:name"));

    iniparser_freedict(d);

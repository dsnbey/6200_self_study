while(1) {

printf("Enter the option to the server. 0-> GET, 1-> PUT, 2-> DELETE: 3 -> TERMINATE THE CLIENT");
scanf("%d", &option);

if (option == 3) {
break;
}

printf("Enter the filename to act upon: ");
bzero(message, MESSAGE_SIZE);
scanf("%s", message);

switch (option) {
case 0:
get(message);
break;
case 1:
put(message);
break;
case 2:
delete(message);
break;
}

}
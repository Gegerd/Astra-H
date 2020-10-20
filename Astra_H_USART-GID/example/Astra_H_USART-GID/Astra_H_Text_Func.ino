//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                       TRANSFORM TEXT AND FUNCTIONS SEND TO DIS                      //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

static byte data[255];  //Data Buffer for Package Preparation
static int data_lenght = 5;  //Useful length of data array for package preparation

//******************** Transliteration array for Cirillic UTF-8  ********************************//

String trans1[48]={"A","B","V","G","D","E","Zh","Z","I","I","K","L","M","N","O","P","R","S","T","U","F","Kh","C","Ch","Sh","Sh","#","I","`","E","Yu","Ya","a","b","v","g","d","e","zh","z","i","i","k","l","m","n","o","p"};
String trans2[16]={"r","s","t","u","f","kh","c","ch","sh","sh","#","i","`","e","yu","ya"};

// ******************* UTF8-Decoder: convert UTF8-string to extended ASCII ***********************
static byte c1;  // Last character buffer
// Convert a single Character from UTF8 to Extended ASCII
// Return "0" if a byte has to be ignored


String utf8ascii(String s)
{
  String r = "";
  char c; 
  byte c1 = 0;	
  for (int i = 0; i < s.length(); i++)
  {	c = s.charAt(i);
	   if ( c < 128 ) // Standard ASCII-set 0..0x7F handling
		{ c1 = 0; r += c;}    // get last char		
  else {
  switch (c1)     // conversion depending on first UTF8-character
  { case 0xC2: r += c;  break;
    case 0xC3: r += (c | 0xC0);  break;
    case 0x82: if (c == 0xAC) r += 0x80;  break; // special case Euro-symbol
    case 0xD0: if (c>=0x90&&c<=0xBF) r += trans1[c-0x90];  break; //Translit Cirillic UTF-8
    case 0xD1: if (c>=0x80&&c<=0x8F) r += trans2[c-0x80];  break;	//Translit Cirillic UTF-8 
	  }
  }
  c1 = c;       // remember actual character
}
  return r;
}

// ******************* Append bytes to the end of the array ***********************
void byte_append(byte add_byte)
{
  if (data_lenght < 254)
  {
    data[data_lenght] = add_byte;
    data_lenght += 1;
  }
}
// ******************* Transform extended ASCII to UTF16 and append text to the end of the byte array ***************
void append_data_str(byte id, String text)
{
  String str = utf8ascii(text);
  int text_len = str.length();
  byte_append(id);
  byte_append(byte(text_len));
  char char_array[text_len + 1];
  str.toCharArray(char_array, text_len + 1);
  for (int i = 0; (i < text_len) & (data_lenght < 254); i++)
  {
    data[data_lenght] = 0x00;
    data[data_lenght + 1] = byte(char_array[i]);
    data_lenght += 2;
  }
}
// ************************** String generation  ****************************
void generate_aux_message(String title)
{
  byte_append(0x03);
  //append_data_str(0x02,"Aux");
  //append_data_str(0x01," ");
  append_data_str(0x10, title);
  //append_data_str(0x11,artist);
  //append_data_str(0x12,album);
  //byte_append(0x01);  //End of message.
  data[4] = byte(data_lenght - 5);
  data[3] = 0x00;
  data[2] = 0xC0;
  data[1] = byte(data_lenght - 2);
  data[0] = 0x10;

}
// ************************** Generation package and send  ****************************
void message_to_DIS (String title)
{
  if ((Prev_Message != title) || Prev_Message == "") {
    data_lenght = 5;
    generate_aux_message(title);
    Prev_Message = title;
  }
  int num = 0;
  int pos = 0;
  byte line[8];
  for (int i = 0; i < data_lenght; i++)
  { if (pos == 8)
    {
      num += 1;
      if (num == 16) num = 0;
      SendCANmessage(MS_MEDIA_ID, 8, line[0], line[1], line[2], line[3], line[4], line[5], line[6], line[7]);
      delay(3); 
      pos = 0;
      line[pos] = byte(num + 32);
      pos += 1;
    }
    line[pos] = data[i];
    pos += 1;
  }
  for (int i = pos; pos < 8; i++)
  {
    line[pos] = 0;
    pos += 1;
  }
  SendCANmessage(MS_MEDIA_ID, 8, line[0], line[1], line[2], line[3], line[4], line[5], line[6], line[7]);
}


//*****************************Text formatting*****************************************//
//**************By default, bold text is displayed with left alignment.****************//
String Bold(String Text) {
  char char_Bold[] = {0x1b, 0x5b, 0x66, 0x53, 0x5f, 0x67, 0x6d, 0x00};
  return (String(char_Bold) + Text);
}
String Normal(String Text) {
  char char_Normal[] = {0x1b, 0x5b, 0x66, 0x53, 0x5f, 0x64, 0x6d, 0x00};
  return (String(char_Normal) + Text);
}
String Right(String Text) {
  char char_Right[] = {0x1b, 0x5b, 0x72, 0x6d, 0x00};
  return (String(char_Right) + Text);
}
String Central(String Text) {
  char char_Central[] = {0x1b, 0x5b, 0x63, 0x6d, 0x00};
  return (String(char_Central) + Text);
}

String data_to_str(int data, int digits) {  //Convert temperature and voltage data to the corresponding type string
  String strf = String(data);
  int y = strf.length() - 1;
  if (digits == 1) {
    char x = strf.charAt(y);
    strf.setCharAt(y, ',');
    strf += (x);
  }
  else strf = strf.substring(0, y);
  return strf;
}

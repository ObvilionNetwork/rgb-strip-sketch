void encrypt(char *plainText, char *cipherText, int16_t length, int16_t *publicKey) {
   int32_t m = 1;
   int16_t n = publicKey[0];
   int16_t e = publicKey[1];
   int16_t ctr = 0;

   for(uint16_t i = 0; i < length; i++) {
       for(int16_t j = 0; j < e; j++) {
           m = (m * plainText[i]) % n;
       }
       
       ctr = i * sizeof(int16_t);

       cipherText[ctr]     = (char) (m & 0x00ff);
       cipherText[ctr + 1] = (char) ((m & 0xff00) >> 8);

       m = 1;
   }
}

void decrypt(char *plainText, char *cipherText, uint16_t length, int16_t *privateKey) {
   int32_t M = 1;
   int16_t n = privateKey[0];
   int16_t d = privateKey[1];
   int16_t temp = 0;
   int16_t ctr = 0;

   for (uint16_t i = 0; i < length; i++) {
       ctr = i * sizeof(int16_t);
       temp = (((unsigned char)cipherText[ctr + 1] << 8) | (unsigned char)cipherText[ctr]);
       
       for (int16_t j = 0; j < d; j++) {
           M = (M * temp) % n;
       }

       plainText[i] = (unsigned char)(M & 0xFF); 
       M = 1;
   }
}

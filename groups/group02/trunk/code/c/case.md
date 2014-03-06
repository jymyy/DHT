Tapaukset
- Odotetaan yhteydenottoja naapureilta
- Avattu yhteys oikeaan/vasempaan
- Kuunnellaan avattua yhteyttää ja odotetaan dataa
    + Odotetaan DHT_REGISTER_ACK jolloin data siirretty
    + ACKin mukana tulee naapurin yhteystiedot
- Kun sekä oikealta että vasemmalta saatu ACK lähetetään serverille DONE
- Kun serveriltä saadaan DONE voidaan unohtaa lähetetty data

Flagit
- Yhteys vasempaan/oikeaan avattu
- Vasemmalta/oikealta on saatu data
- 
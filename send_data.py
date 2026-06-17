import sys
import gspread

def convert(l):
    return [l[0], int(l[1]), l[2], int(l[3]), int(l[4]), l[5], l[6], float(l[7])]

# Récupération des données
f = open(sys.argv[1])
contenu = f.read()
new_rows = [convert(l.split(",")) for l in contenu.split(";")]

# Ajout au fichier gg sheets
gc = gspread.service_account(filename='credentials.json')
sheet = gc.open("Tests TIPE")
worksheet = sheet.worksheet("Données temporelles")

id_new_row = len(worksheet.get("A:H"))

worksheet.insert_rows(new_rows, row=id_new_row+1)



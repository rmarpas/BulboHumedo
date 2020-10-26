 
#!/bin/bash
# $1 = Timestamp desde el que se quiere recibir datos
# $2 numero sensores a imprimir
DIR="./.filtracion"
mili=1000
numParam=$#
#Delete directory .filtracion if exist and create again

echo numero parametros $numParam
tsBegin=$(( $1 * $mili ))

if [ $numParam -eq 2 ]
then
    echo hay 2 parametros
    tsEnd=$(( $(date +%s) * $mili ))
    numberSensors=$2
elif [ $numParam -eq 3 ]
then
    echo Hay 3 parametros
    tsEnd=$(( $2 * $mili ))
    numberSensors=$3
else 
    echo invalid number of parameters
    exit
fi

# Remove .filtracion if exist and create directory again (it deletes all files are include in .filtracion)
if [ -d $DIR ];
then

    rm -rf $DIR
fi
    mkdir $DIR
    
#Delete gnuplot file if exist
if [ -f gnuplot.in ];
then
    rm gnuplot.in
fi

# check if timestamp is valid
# read first timestamp was include in thingsboard
sudo -u postgres -H psql -d thingsboard -c "SELECT ts FROM ts_kv WHERE ts > 0 LIMIT 1" >> ts.txt
sed -i '1,2d' ts.txt
firstTs=$(head -n 1 ts.txt)
rm ts.txt

if [ $tsBegin -lt $firstTs ] || [ $tsEnd -lt $tsBegin ]
then
    echo "Invalid timestamp begin"
    exit
fi

#check that number of sensors is valid
if [ $numberSensors -lt 0 ] || [ $numberSensors -gt 5 ]
then    
    echo "Invalid number of sensors"
    exit
fi


# Saca los datos de la base de datos y los guarda en el archivo tr.txt
for i in $( seq 1 $numberSensors )
do
    sudo -u postgres -H psql -d thingsboard -c "SELECT ts,dbl_v FROM ts_kv WHERE ts BETWEEN "$tsBegin" AND "$tsEnd" AND key LIKE 'Temperatura A-$i'" >> $DIR/A$i.txt
    sed -i '1,2d' $DIR/A$i.txt
    sed -i '/fila/d' $DIR/A$i.txt
done

    #Guardamos la imagen en el archivo ./plot.png
    echo "set output './plot.png'" >> gnuplot.in
    echo "set title \"temperaturas\"" >> gnuplot.in
    echo "set xlabel \"Hora\"" >> gnuplot.in
    echo "set ylabel \"Temperatura"\" >> gnuplot.in
    #Indicamos que el dato de entrada del eje x es un timestamp
    echo "set xdata time" >> gnuplot.in
    echo "set timefmt '%s'" >> gnuplot.in
    #Indicamos que el eje x salga con el fotmato HH:MM:SS
    echo "set format x '%T'" >> gnuplot.in
    echo "set autoscale" >> gnuplot.in
    #indicamos el tipo de línea que va a salir en la gráfica, junto con los colores de las líneas
    echo "set style line 1 lt 1 lc rgb \"sea-green\" lw 2 pointtype 0" >> gnuplot.in
    echo "set style line 2 lt 1 lc rgb \"blue\" lw 2 pointtype 0" >> gnuplot.in
    echo "set style line 3 lt 1  lc rgb \"green\" lw 2 pointtype 0" >> gnuplot.in
    echo "set style line 4 lt 1  lc rgb \"orange\" lw 2 pointtype 0" >> gnuplot.in
    echo "set style line 5 lt 1  lc rgb \"red\" lw 2 pointtype 0" >> gnuplot.in
    # define como separador |
    echo "set datafile separator '|'" >> gnuplot.in
    echo "set terminal png size 1280,720" >> gnuplot.in
    #plot <fichero> using (columnas a utilizar)
    count=1
    
for i in $(ls "$DIR")
do
    #Pasamos de milisegundos a segundos el Timestamp
    while read -r line
    do 
        tsms=$(echo "$line" | cut -d "|" -f 1)
        if [ $tsms > '0' ]
        then
            l=$(expr $tsms / 1000)
            sed -i "s/$tsms/$l /g" "$DIR/$i"
        fi 
    done < "$DIR/$i"
    #incluimos el fichero en gnuplot.in para que se pueda mostrar en la gráfica
    if [ $count = 1 ]
        then
            echo "plot \"$DIR/$i\" using 1:2 with linespoints ls $count title \"Temperaturas-$count\", \\" >> gnuplot.in
    elif [ $count = $numberSensors ]
    then
        echo "\"$DIR/$i\" using 1:2 with linespoints ls $count title \"Temperaturas-$count\"" >> gnuplot.in
        else
            echo "\"$DIR/$i\" using 1:2 with linespoints ls $count title \"Temperaturas-$count\", \\" >> gnuplot.in
        fi
        count=$(expr $count + 1)
done

gnuplot gnuplot.in

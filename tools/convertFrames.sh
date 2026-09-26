# this one i made myself.
for file in $1/*; do
    if [ -f "$file" ]; then 
        echo "Converting $file..."
        fName=$(basename $file)
        echo "Base name: $fName"
        echo "$file" pdc "$fName.pdc"
        pdc_tool "$file" pdc "$1_pdc/$fName.pdc"
    fi 
done

mkdir $1_pdc
python tools/pdc_sequence_builder.py "$1_pdc/" --duration 33 "resources/$(basename $1).pdc"
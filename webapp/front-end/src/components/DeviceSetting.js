import { Typography, Card, Stack, Divider, Switch, TextField } from "@mui/material";


function DeviceSetting(props){

    let valueField;

    if (props.setting_type === "boolean"){
        valueField = <Switch  defaultChecked = {props.value.value} onChange = {(event) => {props.inputReference.current.value = event.target.checked}}/>;
    }
    else if (props.setting_type === "integer"){
        valueField = <TextField id={props.setting_id.toString()} label={props.value.value.toString()} defaultValue = {props.value.value}variant="standard" inputRef = {props.inputReference}/>;
    }

    return (
        <Card style = {{padding:"1%"}}>
            <Stack direction="row" spacing={2} divider={<Divider orientation="vertical" flexItem />}>
                <Typography variant = "h5">{props.setting_id}.</Typography>
                <Typography variant = "h5" sx={{ fontStyle: 'bold' }}>{props.setting_name}</Typography>
                <Typography variant = "h5" sx={{ fontStyle: 'italic' }}>{props.setting_type}</Typography>

                {valueField}                
            </Stack>
        </Card>
    );
}

export default DeviceSetting;
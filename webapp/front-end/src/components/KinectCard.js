import Card from '@material-ui/core/Card'
import CardHeader from '@material-ui/core/CardHeader'
import CardContent from '@material-ui/core/CardContent'
import IconButton from '@material-ui/core/IconButton'
import Typography from '@material-ui/core/Typography'
import { Paper } from '@mui/material'

import { makeStyles } from "@material-ui/core/styles";
import Box from "@material-ui/core/Box";
import { margin } from '@mui/system'
import { Refresh } from '@mui/icons-material'
import FormGroup from '@mui/material/FormGroup';
import FormControlLabel from '@mui/material/FormControlLabel';
import Checkbox from '@mui/material/Checkbox';


export default function KinectCard({ state, handleDelete }) {
    
    let connected = state.connected
    let serial = state.serial
    
    // Style the state
    let  statejsx = <div> 
                      <Paper
                          elevation={2} 
                          style = {{
                            justifyContent: "center",
                            alignItems: "center",
                            textAlign: "center",
                            verticalAlign: "middle",
                            backgroundColor: '#fc9803',
                            margin: 15,
                            borderRadius: 10,
                            width: 200 }} > 
                          <Typography variant="h5" margin={10}>Disconnected</Typography> 
                      </Paper>
                    </div>

    if(connected == 'Y'){
        statejsx = <div> 
                    <Paper
                          elevation={2} 
                          style = {{
                            justifyContent: "center",
                            alignItems: "center",
                            textAlign: "center",
                            verticalAlign: "middle",
                            backgroundColor: '#33FF57',
                            margin: 15,
                            borderRadius: 10,
                            width: 200 }} > 
                      <Typography variant="h5" margin={10}>Connected</Typography> 
                    </Paper>
                </div>
    }

 
    // render
    return (
      <div>

        <Card elevation={2} style = {{backgroundColor: '#fcfcfc'}}>
          
          <CardHeader
            action={
              <IconButton onClick={() => handleDelete()}>
                <Refresh />
              </IconButton>
            }
            title={statejsx}
            subheader={"ID: " + serial}
          />
          
          <CardContent>
            <FormGroup style={{margin:20}}>
              <FormControlLabel control={<Checkbox defaultChecked />} label="Enable Streaming" />
              <FormControlLabel control={<Checkbox />} label="Enable Recording" />
            </FormGroup>
          </CardContent>
       
        </Card>
      </div>
    )
  }
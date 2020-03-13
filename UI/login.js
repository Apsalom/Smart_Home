
var sqlite3 = require('sqlite3');
var express = require('express');
var session = require('express-session');
var bodyParser = require('body-parser');
var path = require('path');

var db=new sqlite3.Database('Smart.db');
var router = express.Router();
var app = express();
var temp;
var value;
const { exec } = require("child_process");
app.use(express.static(__dirname));
app.use(session({
    secret: 'secret',
    resave: true,
    saveUninitialized: true
  }));
app.use(bodyParser.urlencoded({extended : true}));
app.use(bodyParser.json());
router.get('/', function(request, response)
 { 
       
	response.render('index1');
});

router.post('/auth', function(request, response)
     {
           var username1 = request.body.username; 
           var password1 = request.body.password;
           if (username1 && password1)
            {
               db.all('SELECT * FROM Smart  WHERE Username = ? AND Password = ?', [username1, password1], function( error,results) 
                {
                   if (results.length > 0) {
                       response.redirect('/home');
                      }
                   else {
                       response.send('Incorrect Username and/or Password!');
                      }
                   response.end();
               });
           }
           else {
                response.send('Please enter Username and Password!');
                response.end();
               }
      });

router.get('/home', function(request, response)
      {

	var mois;
        var Avail;
        db.serialize(function()
           {
	      db.each('SELECT * FROM Smart',function (err, rows) 
               {
 	         temp=rows.Temp;
        	 mois=rows.mois;
                 value=rows.water;
                 Avail=rows.Avail;
               },function()
                {
  	         response.render('Dashboard',{temp:temp,mois:mois,Avail:Avail});
                });
           });
      });
router.get('/WaterLevel', function(request, response)
 {
      
        response.render('WaterLevel',{value:value});
       
});

router.get('/activity',function(request,response)
 {
       
        exec("./vj", (error, stdout, stderr) => console.log(stdout));     
	
 });
module.exports = router

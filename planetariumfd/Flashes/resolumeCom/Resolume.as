﻿package resolumeCom{	import flash.external.*;	import resolumeCom.parameters.*;	public class Resolume	{				private var publishedParameters:Array;				public function Resolume()		{			this.publishedParameters = new Array();				ExternalInterface.addCallback("getResolumeApiInfo", getResolumeApiInfo);			ExternalInterface.addCallback("getParametersInfo", getParametersInfo);			ExternalInterface.addCallback("setFloatParameter", setFloatParameter);			ExternalInterface.addCallback("setBooleanParameter", setBooleanParameter);						ExternalInterface.addCallback("setEventParameter", setEventParameter);									ExternalInterface.addCallback("setStringParameter", setStringParameter);				}				public function addFloatParameter(name:String, defaultValue:Number) : FloatParameter		{			var parameter:FloatParameter = new FloatParameter(name, defaultValue); 			this.publishedParameters.push(parameter);			return parameter;		}			public function addBooleanParameter(name:String, defaultValue:Boolean) : BooleanParameter		{			var parameter:BooleanParameter = new BooleanParameter(name, defaultValue); 			this.publishedParameters.push(parameter);			return parameter;		}		public function addEventParameter(name:String) : EventParameter		{			var parameter:EventParameter = new EventParameter(name); 			this.publishedParameters.push(parameter);			return parameter;		}				public function addStringParameter(name:String, defaultValue:String) : StringParameter		{			var parameter:StringParameter = new StringParameter(name, defaultValue); 			this.publishedParameters.push(parameter);			return parameter;		}				public function getNumParameters():Number		{			return this.publishedParameters.length;		}			public function getResolumeApiInfo(): String		{			var xmlVersion:String = "<version major='1' minor='0' micro='0' as='3'/>";			return xmlVersion;		}			public function getParametersInfo(): String		{			//create a new array of parameter representations			//Flash automagically serializes it to xml when received in the host			var xmlRep:String = "<params>";			for(var i:Number=0; i<this.publishedParameters.length; ++i) {				var parameter:Parameter = this.publishedParameters[i];						xmlRep += parameter.getXmlRep();			}			xmlRep += "</params>";			return xmlRep;		}		//		public function setFloatParameter(index:Number, value:Number) : Number		{			if (index >=0 && index < this.publishedParameters.length) {				if ( !(this.publishedParameters[index] is FloatParameter) )					return -1;				var parameter:FloatParameter = this.publishedParameters[index];							parameter.setValue(value, true);				return parameter.getValue();					}			return -1;		}		//		public function setBooleanParameter(index:Number, value:Number) : Number		{			if (index >=0 && index < this.publishedParameters.length) {				if ( !(this.publishedParameters[index] is BooleanParameter) )					return 0;				var parameter:BooleanParameter = this.publishedParameters[index];							parameter.setValue( Boolean(value), true );				return Number(parameter.getValue());					}			return 0;		}				public function setEventParameter(index:Number, value:Number) : Number		{			if (index >=0 && index < this.publishedParameters.length) {				if ( !(this.publishedParameters[index] is EventParameter) )					return 0;								var parameter:EventParameter = this.publishedParameters[index];							parameter.setValue( Boolean(value) );				return Number(parameter.getValue());					}			return 0;		}				public function setStringParameter(index:Number, value:String) : String		{			if (index >=0 && index < this.publishedParameters.length) {				if ( !(this.publishedParameters[index] is StringParameter) )					return "";											var parameter:StringParameter = this.publishedParameters[index];							parameter.setValue(value, true);				return parameter.getValue();					}			return "";		}				public function addParameterListener(handler:Function)		{			for(var i:Number=0; i<this.publishedParameters.length; ++i) {				var parameter:Parameter = this.publishedParameters[i];				parameter.addParameterListener(handler);			}		}	}}